/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2013-2019 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DP_SERVER_CLIENT_H
#define DP_SERVER_CLIENT_H

#include "../net/message.h"
#include "jsonapi.h"

#include <QObject>
#include <QTcpSocket>

class QHostAddress;

namespace protocol {
	class MessageQueue;
}

namespace server {

class Session;
class Log;
class ServerLog;

/**
 * @brief Server client
 *
 * This class represents a client that connects to the server.
 * A client is initially in a "lobby" state, until it finishes the login
 * handshake, at which point it is assigned to a session.
 */
class Client : public QObject
{
    Q_OBJECT

public:
	~Client();

	//! Get the user's IP address
	QHostAddress peerAddress() const;

	/**
	 * @brief Assign this client to a session
	 * @param session
	 */
	void setSession(Session *session);
	Session *session();

	/**
	 * @brief Get the context ID of the client
	 *
	 * This is initially zero until the login process is complete.
	 *
	 * The ID is assigned by the session the client is joining, or by the client
	 * itself when hosting a new session
	 * @return client ID
	 */
	uint8_t id() const;
	void setId(uint8_t id);

	/**
	 * @brief Get the user name of this client
	 * @return user name
	 */
	const QString &username() const;
	void setUsername(const QString &username);

	/**
	 * @brief Get this user's avatar.
	 *
	 * The avatar should be a PNG image.
	 * @return
	 */
	const QByteArray &avatar() const;
	void setAvatar(const QByteArray &avatar);

	/**
	 * @brief Get the ext-auth server's ID for this user
	 *
	 * The ext-auth ID is set only if this user logged in via ext-auth,
	 * and the server provided an ID. The ID is only unique within the
	 * same ext-auth server.
	 */
	const QString &extAuthId() const;
	void setExtAuthId(const QString &id);

	/**
	 * @brief Does this user have session operator privileges?
	 * @return
	 */
	bool isOperator() const;
	void setOperator(bool op);

	/**
	 * @brief Is this user a deputy (but not an operator)
	 *
	 * Deputies have more limited permissions than operators.
	 */
	bool isDeputy() const;

	/**
	 * @brief Is this user a moderator?
	 * Moderators can access any session, always have OP status and cannot be kicked by other users.
	 */
	bool isModerator() const;
	void setModerator(bool mod);

	/**
	 * @brief Is this a trusted user?
	 *
	 * The trust flag is granted by session operators. It's effects are purely clientside,
	 * but the server is aware of it so it can remember it for authenticated users.
	 */
	bool isTrusted() const;
	void setTrusted(bool trusted);

	/**
	 * @brief Has this user been authenticated?
	 */
	bool isAuthenticated() const;
	void setAuthenticated(bool auth);

	/**
	 * @brief Has this user been blocked from sending chat messages?
	 */
	bool isMuted() const;
	void setMuted(bool m);

	/**
	 * @brief Set connection idle timeout
	 * @param timeout timeout in milliseconds
	 */
	void setConnectionTimeout(int timeout);

#ifndef NDEBUG
	void setRandomLag(uint lag);
#endif

	enum class DisconnectionReason {
		Kick,     // kicked by an operator
		Error,    // kicked due to some server or protocol error
		Shutdown, // the server is shutting down
	};

	/**
	 * @brief Disconnect this client
	 *
	 * Note. This does not immediately disconnect the client. Instead, a
	 * disconnect command is added to the client's upload queue.
	 *
	 * @param reason
	 * @param message
	 */
	void disconnectClient(DisconnectionReason reason, const QString &message);

	/**
	 * @brief Send a message directly to this client
	 *
	 * Note. Typically messages are sent via the shared session history. Direct
	 * messages are used during the login phase and for client specific notifications.
	 * @param msg
	 */
	void sendDirectMessage(protocol::MessagePtr msg);
	void sendDirectMessage(const protocol::MessageList &msgs);

	/**
	 * @brief Send a message from the server directly to this user
	 * @param message
	 */
	void sendSystemChat(const QString &message);

	/**
	 * @brief Does this client socket support SSL connections?
	 *
	 * Note. This means serverside support. The actual client might not support
	 * SSL.
	 * @return true if server has support for SSL for this client
	 */
	bool hasSslSupport() const;

	/**
	 * @brief Is this connection secure?
	 * @return
	 */
	bool isSecure() const;

	/**
	 * @brief Start SSL handshake
	 */
	void startTls();

	/**
	 * @brief Get a Join message for this user
	 */
	protocol::MessagePtr joinMessage() const;

	/**
	 * @brief Get a JSON object describing this user
	 *
	 * This is used by the admin API
	 */
	QJsonObject description(bool includeSession=true) const;

	/**
	 * @brief Call the client's JSON administration API
	 *
	 * This is used by the HTTP admin API.
	 *
	 * @param method query method
	 * @param path path components
	 * @param request request body content
	 * @return JSON API response content
	 */
	JsonApiResult callJsonApi(JsonApiMethod method, const QStringList &path, const QJsonObject &request);

	/**
	 * @brief Divert incoming messages to a holding buffer
	 *
	 * When hold-locked, all incoming messages are saved in a buffer
	 * until the lock is released.
	 *
	 * @param lock
	 */
	void setHoldLocked(bool lock);
	bool isHoldLocked() const;

	/**
	 * @brief Write a log entry
	 *
	 * The user and session fields are filled in automatically.
	 * Note: this can only be used after the user has joined a session.
	 */
	void log(Log entry) const;
	Log log() const;

signals:
	/**
	 * @brief Message received while not part of a session
	 */
	void loginMessage(protocol::MessagePtr message);

	/**
	 * @brief This client is disconnecting
	 *
	 * This signal may be emitted twice: first when the client
	 * is instructed to disconnect (e.g. kicked)
	 * and the second time when the socket actually disconnects.
	 */
	void loggedOff(Client *client);

private slots:
	void gotBadData(int len, int type);
	void receiveMessages();
	void socketError(QAbstractSocket::SocketError error);
	void socketDisconnect();

protected:
	Client(QTcpSocket *socket, ServerLog *logger, QObject *parent);
	protocol::MessageQueue *messageQueue();

private:
	void handleSessionMessage(protocol::MessagePtr msg);

	struct Private;
	Private *d;
};

}

#endif

