#ifndef TEXTOUTPUTLABEL_H
#define TEXTOUTPUTLABEL_H
#include <QLabel>

class TextoutputLabel : public QLabel
{
public:
	TextoutputLabel();
	virtual void setText(const QString &text);


};

#endif // TEXTOUTPUTLABEL_H
