import QtQuick 2.1
import QtQuick.Layouts 1.3
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
	id: root
	readonly property alias pm_data: pm_source.data

	readonly property bool   st_battery_has: pm_data['Battery']['Has Battery']
	readonly property string st_battery_state: pm_data['Battery']['State']
	readonly property int    st_battery_percent: pm_data['Battery']['Percent']

	// https://github.com/KDE/plasma-workspace/blob/master/dataengines/powermanagement/powermanagementengine.h
	// https://github.com/KDE/plasma-workspace/blob/master/dataengines/powermanagement/powermanagementengine.cpp
	PlasmaCore.DataSource {
		id: pm_source
		engine: 'powermanagement'
		connectedSources: ['Battery'] // basicSourceNames == ["Battery", "AC Adapter", "Sleep States", "PowerDevil", "Inhibitions"]
		function log() {
			for (var i = 0; i < pm_source.sources.length; i++) {
				var sourceName = pm_source.sources[i]
				var source = pm_source.data[sourceName]
				for (var key in source) {
					console.error('pm_source.data["'+sourceName+'"]["'+key+'"] =', source[key])
				}
			}
		}
	}
	// Component.onCompleted: pm_source.log()
}
