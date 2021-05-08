#include "textoutputlabel.h"

TextoutputLabel::TextoutputLabel()
{
	QFont font;
	font.setPointSize(48);
	font.setBold(true);
	this->setFont(font);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->setAlignment(Qt::AlignCenter);
	this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
}

void TextoutputLabel::setText(const QString &text)
{
	QLabel::setText(text);
	QFont currentFont = this->font();
	int calculatedPointSize = currentFont.pointSize();
	QFontMetrics fm(currentFont);
	int contentWidth = this->contentsRect().width() - this->margin();
	while(calculatedPointSize < 48 && fm.boundingRect(this->text()).width() < contentWidth)
	{
		calculatedPointSize += 1;
		currentFont.setPointSize(calculatedPointSize);
		fm = QFontMetrics(currentFont);
	}
	while(fm.boundingRect(this->text()).width() >= contentWidth)
	{
		calculatedPointSize -= 1;
		currentFont.setPointSize(calculatedPointSize);
		fm = QFontMetrics(currentFont);
	}
	this->setFont(currentFont);
}



