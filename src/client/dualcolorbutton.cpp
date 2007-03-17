#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QDrag>

#include "dualcolorbutton.h"

#ifndef DESIGNER_PLUGIN
namespace widgets {
#endif

DualColorButton::DualColorButton(QWidget *parent)
	: QWidget(parent), foreground_(Qt::black), background_(Qt::white)
{
	setMinimumSize(32,32);
	setAcceptDrops(true);
}

DualColorButton::DualColorButton(const QColor& fgColor, const QColor& bgColor, QWidget *parent)
	: QWidget(parent), foreground_(fgColor), background_(bgColor)
{
	setMinimumSize(32,32);
	setAcceptDrops(true);
}

/**
 * The foregroundColorChanged signal is emitted
 * @param c color to set
 */
void DualColorButton::setForeground(const QColor &c)
{
	foreground_ = c;
	emit foregroundChanged(c);
	update();
}

/**
 * The backgroundColorChanged signal is emitted
 * @param c color to set
 */
void DualColorButton::setBackground(const QColor &c)
{
	background_ = c;
	emit backgroundChanged(c);
	update();
}

/**
 * Foreground and background colors switch places and signals are emitted.
 */
void DualColorButton::swapColors()
{
	QColor tmp = foreground_;
	foreground_ = background_;
	background_ = tmp;
	emit foregroundChanged(foreground_);
	emit backgroundChanged(background_);
	update();
}

QRect DualColorButton::foregroundRect() const
{
	// foreground rectangle fills the upper left two thirds of the widget
	return QRect(0,0, qRound(width()/3.0*2), qRound(height()/3.0*2));
}

QRect DualColorButton::backgroundRect() const
{
	// Background rectangle filles the lower right two thirds of the widget.
	// It is partially obscured by the foreground rectangle
	int x = qRound(width() / 3.0);
	int y = qRound(height() / 3.0);
	return QRect(x-1,y-1, x*2-1, y*2-1);
}

QRect DualColorButton::resetBlackRect() const
{
	int x = qRound(width()/9.0);
	int y = qRound(height()/9.0*7);
	int w = qRound(width() / 9.0);
	int h = qRound(height() / 9.0);
	return QRect(x-w/3,y-h/3,w,h);
}

QRect DualColorButton::resetWhiteRect() const
{
	QRect r = resetBlackRect();
	r.translate(r.width()/2, r.height()/2);
	return r;
}

void DualColorButton::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setPen(QPen(Qt::black));

	// Draw background box
	QRect bgbox = backgroundRect();
	painter.fillRect(bgbox, background_);
	painter.drawRect(bgbox);

	// Draw foreground box
	QRect fgbox = foregroundRect();
	painter.fillRect(fgbox, foreground_);
	painter.drawRect(fgbox);

	// Draw reset boxes
	QRect rwhite = resetWhiteRect();
	painter.fillRect(rwhite, Qt::white);
	painter.drawRect(rwhite);
	painter.fillRect(resetBlackRect(), Qt::black);

	// Draw swap arrow
	static const QPoint arrows[12] = {
		QPoint(3,1),
		QPoint(1,3),
		QPoint(3,5),
		QPoint(3,4),
		QPoint(6,4),
		QPoint(6,7),
		QPoint(5,7),
		QPoint(7,9),
		QPoint(9,7),
		QPoint(8,7),
		QPoint(8,2),
		QPoint(3,2)
	};

	painter.scale(width()/10.0/3.0, height()/10.0/3.0);
	painter.translate(20,0);
	painter.setBrush(palette().light());
	painter.drawConvexPolygon(arrows,12);

}

void DualColorButton::mousePressEvent(QMouseEvent *event)
{
	if(event->button() != Qt::LeftButton)
		return;
	dragSource_ = NODRAG;
	if(backgroundRect().contains(event->pos()))
			dragSource_ = BACKGROUND;
	if(foregroundRect().contains(event->pos()))
			dragSource_ = FOREGROUND;

	dragStart_ = event->pos();
}

void DualColorButton::mouseMoveEvent(QMouseEvent *event)
{
	if(dragSource_ != NODRAG && (event->buttons() & Qt::LeftButton) &&
			(event->pos() - dragStart_).manhattanLength()
		< QApplication::startDragDistance())
	{
		QDrag *drag = new QDrag(this);

		QMimeData *mimedata = new QMimeData;
		QColor color = (dragSource_ == FOREGROUND)?foreground_:background_;
		mimedata->setColorData(color);

		drag->setMimeData(mimedata);
		drag->start(Qt::CopyAction);
	}
}

void DualColorButton::mouseReleaseEvent(QMouseEvent *event)
{
	QRect swaprect(qRound(width()*2.0/3.0),0,width()/3,height()/3);
	if(resetBlackRect().contains(event->pos()) || resetWhiteRect().contains(event->pos())) {
		foreground_ = Qt::black;
		background_ = Qt::white;
		emit foregroundChanged(foreground_);
		emit backgroundChanged(background_);
		update();
	} else if(foregroundRect().contains(event->pos())) {
		emit foregroundClicked();
	} else if(backgroundRect().contains(event->pos())) {
		emit backgroundClicked();
	} else if(swaprect.contains(event->pos())) {
		swapColors();
	}
}

void DualColorButton::dragEnterEvent(QDragEnterEvent *event)
{
	if(event->mimeData()->hasFormat("application/x-color"))
		event->acceptProposedAction();
}

void DualColorButton::dropEvent(QDropEvent *event)
{
	QColor color = qvariant_cast<QColor>(event->mimeData()->colorData());
	if(foregroundRect().contains(event->pos())) {
		foreground_ = color;
		emit foregroundChanged(color);
	} else if(backgroundRect().contains(event->pos())) {
		background_ = color;
		emit backgroundChanged(color);
	}
	update();
}

#ifndef DESIGNER_PLUGIN
}
#endif

