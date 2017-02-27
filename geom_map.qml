import QtGraphicalEffects 1.0
import QtQuick 2.5
import QtQuick.Window 2.0
import QtQuick.Controls 2.0
import QtPositioning 5.6
import QtLocation 5.6




Window {
    width: 300
    height: 300
    visible: true

    Plugin {
        id: osmPlugin
        name: "osm"
        // specify plugin parameters if necessary
        // PluginParameter {
        //     name:
        //     value:
        // }
    }

    Map {
        anchors.fill: parent
        plugin: osmPlugin
        center: QtPositioning.coordinate(51.041025, -114.079155) // Oslo
        zoomLevel: 10
    }
}








//}
