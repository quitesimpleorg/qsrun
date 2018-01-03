#ifndef CALCULATIONENGINE_H
#define CALCULATIONENGINE_H
#include <QString>
#include <libqalculate/Calculator.h>
#include <libqalculate/ExpressionItem.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Prefix.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>

class CalculationEngine
{
private:
    EvaluationOptions eo;
    PrintOptions po;

public:
    CalculationEngine();
    QString evaluate(const QString &expression);
};

#endif // CALCULATIONENGINE_H
