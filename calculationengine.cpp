#include "calculationengine.h"


CalculationEngine::CalculationEngine()
{
	if (!CALCULATOR) {
		   new Calculator();
		   CALCULATOR->terminateThreads();
			CALCULATOR->setPrecision(16);
   }

	//stolen from krunner's qalculate engine
	/*eo.auto_post_conversion = POST_CONVERSION_BEST;
	eo.keep_zero_units = false;

	eo.parse_options.angle_unit = ANGLE_UNIT_RADIANS;
	eo.structuring = STRUCTURING_SIMPLIFY;

	// suggested in https://github.com/Qalculate/libqalculate/issues/16
	// to avoid memory overflow for seemingly innocent calculations (Bug 277011)
	eo.approximation = APPROXIMATION_APPROXIMATE;

	po.number_fraction_format = FRACTION_DECIMAL;
	po.indicate_infinite_series = false;
	po.use_all_prefixes = false;
	po.use_denominator_prefix = true;
	po.negative_exponents = false;
	po.lower_case_e = true;
	po.base_display = BASE_DISPLAY_NORMAL;*/
}

QString CalculationEngine::evaluate(const QString &expression)
{
	CALCULATOR->terminateThreads();
	QByteArray ba = expression.toLatin1();
	const char *ctext = ba.data();
	MathStructure result = CALCULATOR->calculate(ctext, this->eo);
	 result.format(po);
	return result.print(po).c_str();


}
