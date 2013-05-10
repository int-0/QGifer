#include <QPainter>
#include <QColor>
#include <QColorDialog>
#include "textwidget.h"

TextWidget::TextWidget(FramePlayer* fp, QWidget* parent, Qt::WindowFlags f): 
     QWidget(parent,f), textObject(NULL), editMode(false)
{
     init();
     setPlayer(fp);
     lineEdit->setText("QGifer");
     tcEdit->setText("#FFFFFF");
     ocEdit->setText("#000000");
     posButton->setText(tr("Insert"));
     negButton->setText(tr("Cancel"));
     connect(posButton, SIGNAL(clicked()), this, SLOT(insert()));
     validate();
}

TextWidget::TextWidget(TextObject* to, FramePlayer* fp, QWidget* parent, Qt::WindowFlags f): 
     QWidget(parent,f), editMode(true)
{
     init();
     textObject = to;
     setPlayer(fp);
     
     lineEdit->setText(textObject->getText());
     tcEdit->setText(textObject->getTextColor().name());
     ocEdit->setText(textObject->getOutlineColor().name());
     outlineBox->setValue(textObject->getOutlineWidth());
     sizeBox->setValue(textObject->getFont().pointSize());
     fontComboBox->setCurrentFont(textObject->getFont());
     boldBox->setChecked( textObject->getFont().bold() );
     italicBox->setChecked( textObject->getFont().italic() );
     fromBox->setValue( textObject->getStart() );
     toBox->setValue( textObject->getStop() );

     posButton->setText(tr("Apply"));
     negButton->setText(tr("Close"));
     connect(posButton, SIGNAL(clicked()), this, SLOT(apply()));
     validate();
}


TextWidget::~TextWidget()
{
     
}

void TextWidget::init()
{
     qDebug() << "initializing text widget...";
     setupUi(this);
     connect(sizeBox, SIGNAL(valueChanged(int)), this, SLOT(update()));
     connect(outlineBox, SIGNAL(valueChanged(double)), this, SLOT(update()));
     connect(boldBox, SIGNAL(stateChanged(int)), this, SLOT(update()));
     connect(italicBox, SIGNAL(stateChanged(int)), this, SLOT(update()));
     connect(lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(update()));
     connect(lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(validate()));
     connect(tcEdit, SIGNAL(textChanged(const QString&)), this, SLOT(update()));
     connect(ocEdit, SIGNAL(textChanged(const QString&)), this, SLOT(update()));
     connect(fontComboBox, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(update()));
     connect(curFromButton, SIGNAL(clicked()), this, SLOT(fromUpdate()));
     connect(curToButton, SIGNAL(clicked()), this, SLOT(toUpdate()));

     connect(tcButton, SIGNAL(clicked()), this, SLOT(setTColor()));
     connect(ocButton, SIGNAL(clicked()), this, SLOT(setOColor()));
     connect(fromBox, SIGNAL(valueChanged(int)), this, SLOT(validate()));     
     connect(toBox, SIGNAL(valueChanged(int)), this, SLOT(validate()));
     connect(negButton, SIGNAL(clicked()), this, SLOT(close()));
     qDebug() << "done...";
}

void TextWidget::paintEvent(QPaintEvent*)
{
     QPainter p(this);
     QImage i = renderText();
     if(!i.isNull())
	  p.drawImage( frame->x()+(frame->width()-i.width())/2, frame->y()+(frame->height()-i.height())/2 , i);
}

QImage TextWidget::renderText() const
{
     if(lineEdit->text().isEmpty())
	  return QImage();
     QPen pen;
     pen.setColor(QColor(ocEdit->text()));
     pen.setWidthF(outlineBox->value());
    
     QFont font = fontComboBox->currentFont();
     font.setPointSize(sizeBox->value());
     font.setItalic(italicBox->isChecked());
     font.setBold(boldBox->isChecked());

     QFontMetrics fm(font);
     QRect r = fm.boundingRect(lineEdit->text());

     QPainterPath path;
     path.addText(r.width()*0.05, fm.height(), font, lineEdit->text());

     //QPixmap pix(QSize(path.boundingRect().width(), path.boundingRect().height()));
     QPixmap pix(QSize(r.width()*1.1f, r.height()*1.3f));
     pix.fill(Qt::transparent);
     QImage img = pix.toImage();

     QPainter p(&img);
     p.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing, true);

     p.setFont(font);
     p.setPen(pen);
     p.setBrush(QColor(tcEdit->text()));
     p.drawPath(path);

     return img;
}

void TextWidget::insert()
{
     textObject = new TextObject;
     apply();
}

void TextWidget::apply()
{
     TextObject* o = textObject;
     o->setText(lineEdit->text());
     QFont f = fontComboBox->currentFont();
     f.setPointSize(sizeBox->value());
     f.setBold( boldBox->isChecked() );
     f.setItalic( italicBox->isChecked() );
     o->setFont(f);
     o->setTextColor( QColor(tcEdit->text()) );
     o->setOutlineColor( QColor(ocEdit->text()) );
     o->setOutlineWidth( outlineBox->value() );
     o->setRange( fromBox->value(), toBox->value() );
     o->setImage( renderText() );
     if(!editMode)
	  player->getWorkspace()->addObject(o);
     close();
}

void TextWidget::setTColor()
{
     QColor c = QColorDialog::getColor(QColor(tcEdit->text()), this, tr("Set text color"));
     if(c.isValid())
	  tcEdit->setText(c.name());
}

void TextWidget::setOColor()
{
     QColor c = QColorDialog::getColor(QColor(ocEdit->text()), this, tr("Set outline color"));
     if(c.isValid())
	  ocEdit->setText(c.name());
}

void TextWidget::setPlayer(FramePlayer* fp)
{
     player = fp;
     fromBox->setMaximum(player->countFrames());
     toBox->setMaximum(player->countFrames());
}

void TextWidget::validate()
{
     posButton->setEnabled(!lineEdit->text().isEmpty() && 
			   fromBox->value() <= toBox->value() && 
			   player->countFrames());
     
}

void TextWidget::fromUpdate()
{
     if(player)
	  fromBox->setValue(player->getCurrentPos());
     validate();
}

void TextWidget::toUpdate()
{
     if(player)
	  toBox->setValue(player->getCurrentPos());
     validate();
}
