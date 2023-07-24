// SPDX-License-Identifier: GPL-3.0-or-later

#include "libclient/utils/news.h"
#include "cmake-config/config.h"
#include "libclient/utils/database.h"
#include "libshared/util/networkaccess.h"
#include <QDate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLoggingCategory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimer>
#include <functional>


Q_LOGGING_CATEGORY(lcDpNews, "net.drawpile.news", QtWarningMsg)

namespace utils {

News::Version News::Version::parse(const QString &s)
{
	static QRegularExpression re{
		"\\A([0-9]+)\\.([0-9+])\\.([0-9]+)(?:-beta\\.([0-9]+))?"};

	QRegularExpressionMatch match = re.match(s);
	if(!match.hasMatch()) {
		return invalid();
	}

	News::Version version;
	bool ok;
	version.server = match.captured(1).toInt(&ok);
	if(!ok) {
		return invalid();
	}

	version.major = match.captured(2).toInt(&ok);
	if(!ok) {
		return invalid();
	}

	version.minor = match.captured(3).toInt(&ok);
	if(!ok) {
		return invalid();
	}

	QString betaStr = match.captured(4);
	if(betaStr.isEmpty()) {
		version.beta = 0;
	} else {
		version.beta = betaStr.toInt(&ok);
		if(!ok) {
			return invalid();
		}
	}

	return version;
}

bool News::Version::isNewerThan(const Version &other) const
{
	if(server > other.server) {
		return true;
	} else if(server == other.server) {
		if(major > other.major) {
			return true;
		} else if(major == other.major) {
			if(minor > other.minor) {
				return true;
			} else if(minor == other.minor) {
				if(beta > other.beta) {
					return true;
				}
			}
		}
	}
	return false;
}

QString News::Version::toString() const
{
	QString s = QStringLiteral("%1.%2.%3").arg(server).arg(major).arg(minor);
	return isBeta() ? QStringLiteral("%1-beta.%2").arg(s).arg(beta) : s;
}

class News::Private {
public:
	Private()
		: m_db{utils::db::sqlite(
			  QStringLiteral("drawpile_state_connection"),
			  QStringLiteral("state"), QStringLiteral("state.db"))}
	{
		initDb();
	}

	bool fetching() const { return m_fetching; }
	void setFetching(bool fetching) { m_fetching = fetching; }

	static QString dateToString(const QDate &date)
	{
		return date.isValid() ? date.toString(Qt::ISODate)
							  : QStringLiteral("0000-00-00");
	}

	static QDate dateFromString(const QString &s)
	{
		if(s.isEmpty() || s == QStringLiteral("0000-00-00")) {
			return QDate{};
		} else {
			return QDate::fromString(s, Qt::ISODate);
		}
	}

	QString readNewsContentFor(const QDate &date)
	{
		QSqlQuery query(m_db);
		bool hasRow = utils::db::exec(
						  query,
						  "select content from news where date <= ?\n"
						  "order by date desc limit 1",
						  {dateToString(date)}) &&
					  query.next();
		return hasRow ? query.value(0).toString() : QString{};
	}

	bool isStale(const QDate &date, long long staleDays)
	{
		QDate lastCheck = readLastCheck();
		if(lastCheck.isValid()) {
			long long dayDiff = lastCheck.daysTo(date);
			qCDebug(lcDpNews, "Day delta %lld", dayDiff);
			// We'll use the absolute value of days, since having checked for
			// news *in the future* means there's something funky going on.
			return qAbs(dayDiff) >= staleDays;
		} else {
			return true;
		}
	}

	Update readUpdateAvailable(const Version &forVersion, bool includeBeta)
	{
		QSqlQuery query(m_db);
		if(utils::db::exec(
			   query, "select server, major, minor, beta, date, url\n"
					  "from updates\n"
					  "order by server, major, minor, beta desc")) {
			while(query.next()) {
				Version candidate = {
					query.value(0).toInt(), query.value(1).toInt(),
					query.value(2).toInt(), query.value(3).toInt()};
				if(candidate.isNewerThan(forVersion)) {
					if(includeBeta || !candidate.isBeta()) {
						qCDebug(
							lcDpNews, "Candidate version %s > %s",
							qUtf8Printable(candidate.toString()),
							qUtf8Printable(forVersion.toString()));
						return {
							candidate,
							dateFromString(query.value(4).toString()),
							QUrl{query.value(5).toString()}};
					} else {
						qCDebug(
							lcDpNews, "Candidate version %s > %s, but is beta",
							qUtf8Printable(candidate.toString()),
							qUtf8Printable(forVersion.toString()));
					}
				} else {
					qCDebug(
						lcDpNews, "Candidate version %s <= %s",
						qUtf8Printable(candidate.toString()),
						qUtf8Printable(forVersion.toString()));
					break;
				}
			}
		}
		return {Version::invalid(), QDate{}, QUrl{}};
	}

	bool writeNewsAt(const QVector<News::Article> articles, const QDate &today)
	{
		return utils::db::tx(m_db, [&] {
			QSqlQuery query{m_db};
			if(!utils::db::exec(query, QStringLiteral("delete from news"))) {
				return false;
			}

			QString sql = QStringLiteral(
				"insert into news (content, date) values (?, ?)");
			if(!utils::db::prepare(query, sql)) {
				return false;
			}

			for(const News::Article &article : articles) {
				query.bindValue(0, article.content);
				query.bindValue(1, dateToString(article.date));
				if(!utils::db::execPrepared(query, sql)) {
					return false;
				}
			}

			return putStateOn(query, LAST_CHECK_KEY, dateToString(today));
		});
	}

	bool writeUpdates(const QVector<News::Update> updates)
	{
		return utils::db::tx(m_db, [&] {
			QSqlQuery query{m_db};
			if(!utils::db::exec(query, QStringLiteral("delete from updates"))) {
				return false;
			}

			QString sql = QStringLiteral(
				"insert into updates (server, major, minor, beta, date, url)\n"
				"values (?, ?, ?, ?, ?, ?)");
			if(!utils::db::prepare(query, sql)) {
				return false;
			}

			for(const News::Update &update : updates) {
				query.bindValue(0, update.version.server);
				query.bindValue(1, update.version.major);
				query.bindValue(2, update.version.minor);
				query.bindValue(3, update.version.beta);
				query.bindValue(4, dateToString(update.date));
				query.bindValue(5, update.url.toString());
				if(!utils::db::execPrepared(query, sql)) {
					return false;
				}
			}

			return true;
		});
	}

	QDate readLastCheck() const
	{
		return dateFromString(getStateOn(LAST_CHECK_KEY).toString());
	}

private:
	static constexpr char LAST_CHECK_KEY[] = "news:lastcheck";

	void initDb()
	{
		QSqlQuery query{m_db};
		utils::db::exec(query, "pragma foreign_keys = on");
		utils::db::exec(
			query, "create table if not exists migrations (\n"
				   "	migration_id integer primary key not null)");
		utils::db::exec(
			query, "create table if not exists state (\n"
				   "	key text primary key not null,\n"
				   "	value)");
		utils::db::exec(
			query, "create table if not exists news (\n"
				   "	content text not null,\n"
				   "	date text not null)");
		utils::db::exec(
			query, "create table if not exists updates (\n"
				   "	server integer not null,\n"
				   "	major integer not null,\n"
				   "	minor integer not null,\n"
				   "	beta integer not null,\n"
				   "	date text not null,\n"
				   "	url text not null)");
	}

	bool putStateOn(QSqlQuery &query, const QString &key, const QVariant &value)
	{
		QString sql =
			QStringLiteral("insert into state (key, value) values (?, ?)\n"
						   "	on conflict (key) do update set value = ?");
		return utils::db::exec(query, sql, {key, value, value});
	}

	QVariant getStateOn(const QString &key) const
	{
		QSqlQuery query(m_db);
		return getStateWith(query, key);
	}

	QVariant getStateWith(QSqlQuery &query, const QString &key) const
	{
		bool hasRow =
			utils::db::exec(
				query, "select value from state where key = ?", {key}) &&
			query.next();
		return hasRow ? query.value(0) : QVariant{};
	}

	QSqlDatabase m_db;
	bool m_fetching = false;
};

News::News(QObject *parent)
	: QObject{parent}
	, d{new Private}
{
}

News::~News()
{
	delete d;
}

void News::check()
{
	emit fetchInProgress(true);
	doCheck(false);
}

void News::forceCheck(int delayMsec)
{
	emit fetchInProgress(true);
	showMessageAsNews(tr("Checking for updates…"));
	QTimer::singleShot(delayMsec, this, std::bind(&News::doCheck, this, true));
}

void News::checkExisting()
{
	QDate date = QDate::currentDate();
	if(!d->isStale(date, CHECK_EXISTING_STALE_DAYS)) {
		QString content = d->readNewsContentFor(date);
		if(!content.isEmpty()) {
			emit newsAvailable(content);
			return;
		}
	}
	emit newsAvailable(
		QStringLiteral("<p style=\"font-size:large;\">%1</p>"
					   "<p style=\"font-size:large;\">%2</p>")
			.arg(
				tr("Automatic update checking is disabled, "
				   "<a href=\"#autoupdate\">click here to enable it</a>."),
				tr("If you don't want automatic checks, "
				   "<a href=\"#checkupdates\">click here to check "
				   "manually</a>.")));
}

QDate News::lastCheck() const
{
	return d->readLastCheck();
}

void News::doCheck(bool force)
{
	QDate date = QDate::currentDate();
	if(force) {
		qCDebug(lcDpNews, "Force fetching news");
		fetch(date, true);
	} else {
		QString content = d->readNewsContentFor(date);
		if(content.isEmpty()) {
			qCDebug(lcDpNews, "Fetching initial news");
			if(!force) {
				showMessageAsNews(tr("Checking for updates…"));
			}
			fetch(date, true);
		} else {
			checkUpdateAvailable();
			emit newsAvailable(content);
			if(d->isStale(date, CHECK_STALE_DAYS)) {
				qCDebug(lcDpNews, "Fetching news");
				fetch(date, false);
			} else {
				qCDebug(lcDpNews, "Not fetching news");
				emit fetchInProgress(false);
			}
		}
	}
}

void News::fetch(const QDate &date, bool showErrorAsNews)
{
	d->setFetching(true);
	QNetworkRequest req(QUrl{URL});
	QNetworkReply *reply = networkaccess::getInstance()->get(req);
	connect(reply, &QNetworkReply::finished, this, [=] {
		fetchFinished(date, reply, showErrorAsNews);
		qCDebug(lcDpNews, "Done fetching news");
		d->setFetching(false);
		emit fetchInProgress(false);
	});
	connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

void News::fetchFinished(QDate date, QNetworkReply *reply, bool showErrorAsNews)
{
	if(reply->error() != QNetworkReply::NoError) {
		qCWarning(
			lcDpNews, "Network error fetching news: %s",
			qUtf8Printable(reply->errorString()));
		if(showErrorAsNews) {
			showMessageAsNews(
				tr("Network error: %1").arg(reply->errorString()));
		}
		return;
	}

	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err);
	if(doc.isNull() && err.error != QJsonParseError::NoError) {
		qCWarning(
			lcDpNews, "Error parsing news: %s",
			qUtf8Printable(err.errorString()));
		if(showErrorAsNews) {
			showMessageAsNews(tr("Couldn't make sense of the fetched data."));
		}
		return;
	}

	QVector<News::Update> updates = parseUpdates(doc);
	if(updates.isEmpty()) {
		if(showErrorAsNews) {
			showMessageAsNews(tr("Couldn't make sense of fetched updates."));
		}
		return;
	}

	if(!d->writeUpdates(updates)) {
		if(showErrorAsNews) {
			showMessageAsNews(tr("Couldn't save updates."));
		}
		return;
	}

	checkUpdateAvailable();

	QVector<News::Article> articles = parseNews(doc);
	if(articles.isEmpty()) {
		if(showErrorAsNews) {
			showMessageAsNews(tr("Couldn't make sense of fetched news."));
		}
		return;
	}

	if(!d->writeNewsAt(articles, date)) {
		if(showErrorAsNews) {
			showMessageAsNews(tr("Couldn't save news."));
		}
		return;
	}

	QString content = d->readNewsContentFor(date);
	if(content.isEmpty()) {
		if(showErrorAsNews) {
			showMessageAsNews(tr("No news available."));
		}
		return;
	}

	emit newsAvailable(content);
}

QVector<News::Article> News::parseNews(const QJsonDocument &doc)
{
	QJsonValue input = doc["news"];
	if(!input.isArray()) {
		qCWarning(lcDpNews, "News element is not an array");
		return {};
	}

	QVector<Article> articles;
	for(QJsonValue value : input.toArray()) {
		QString content = value["content"].toString();
		if(content.isEmpty()) {
			qCWarning(lcDpNews, "News article content is empty");
			continue;
		}

		QString dateString = value["date"].toString();
		QDate date;
		if(!dateString.isEmpty()) {
			date = Private::dateFromString(dateString);
			if(!date.isValid()) {
				qCWarning(lcDpNews, "News article date is invalid");
				continue;
			}
		}

		articles.append({content, date});
	}

	if(articles.isEmpty()) {
		qCWarning(lcDpNews, "No news articles in response");
	}
	return articles;
}

QVector<News::Update> News::parseUpdates(const QJsonDocument &doc)
{
	QJsonValue input = doc["updates"];
	if(!input.isArray()) {
		qCWarning(lcDpNews, "Updates element is not an array");
		return {};
	}

	QVector<Update> updates;
	for(QJsonValue value : input.toArray()) {
		QString versionString = value["version"].toString();
		Version version = Version::parse(versionString);
		if(!version.isValid()) {
			qCWarning(
				lcDpNews, "Invalid update version '%s'",
				qUtf8Printable(versionString));
			continue; // No fallback for an invalid version.
		}

		QString urlString = value["url"].toString();
		QUrl url{urlString};
		if(!url.isValid()) {
			qCWarning(
				lcDpNews, "Invalid update url '%s'", qUtf8Printable(urlString));
			url = FALLBACK_UPDATE_URL;
		}

		QString dateString = value["date"].toString();
		QDate date = Private::dateFromString(dateString);
		if(!date.isValid()) {
			qCWarning(
				lcDpNews, "Invalid update date '%s'",
				qUtf8Printable(dateString));
			// Keep going, we'll use 0000-00-00 as a date.
		}

		updates.append({version, date, url});
	}

	if(updates.isEmpty()) {
		qCWarning(lcDpNews, "No updates in response");
	}
	return updates;
}

void News::checkUpdateAvailable()
{
	QString currentVersionStr{cmake_config::version()};
	Version currentVersion = Version::parse(currentVersionStr);
	if(currentVersion.isValid()) {
		Update update =
			d->readUpdateAvailable(currentVersion, currentVersion.isBeta());
		emit updateAvailable(update);
	} else {
		qCWarning(
			lcDpNews, "Current version '%s' is invalid",
			qUtf8Printable(currentVersionStr));
	}
}

void News::showMessageAsNews(const QString &message)
{
	emit newsAvailable(QStringLiteral("<p style=\"font-size:large;\">%1</p>")
						   .arg(message.toHtmlEscaped()));
}

}
