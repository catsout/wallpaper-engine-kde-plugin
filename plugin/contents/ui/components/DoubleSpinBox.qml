import QtQuick 2.8
import QtQuick.Controls 2.1

SpinBox {
	id: dSpBox

	property int decimals: 2
	property double dFrom: 0.0
	property double dTo: 100.0
	property double dValue: 0.0
	property double dStepSize: 0.1

	readonly property int factor: Math.pow(10, dSpBox.decimals)

	stepSize: dSpBox.dStepSize * dSpBox.factor
	from: dSpBox.dFrom * dSpBox.factor
	to: dSpBox.dTo * dSpBox.factor
	value: dSpBox.dValue * dSpBox.factor

	validator: DoubleValidator {
		bottom: dSpBox.dFrom
		top: dSpBox.dTo
	}

	textFromValue: function(value, locale) {
		return Number(value / dSpBox.factor).toLocaleString(locale, 'f', dSpBox.decimals);
	}

	valueFromText: function(text, locale) {
		return Number.fromLocaleString(locale, text) * dSpBox.factor;
	}

	onDValueChanged: dSpBox.value = dSpBox.dValue * dSpBox.factor
	onValueChanged: dSpBox.dValue = dSpBox.value / dSpBox.factor
}
