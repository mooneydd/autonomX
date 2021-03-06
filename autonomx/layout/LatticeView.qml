import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import ca.hexagram.xmodal.autonomx 1.0

import "qrc:/stylesheet"
import "../components/ui"

RowLayout {
    id: latticeView

    // props
    property int generatorIndex: window.activeGeneratorIndex
    property GeneratorRegionModel inputModel: generatorIndex < 0 ? null : generatorModel.at(generatorIndex).getInputRegionModel()
    property GeneratorRegionModel outputModel: generatorIndex < 0 ? null : generatorModel.at(generatorIndex).getOutputRegionModel()
    property int ppc: 20            // pixels per cell, ie. how wide a cell square is in pixels. this is animated within QML (scaled by the zoom factor)
    property QtObject currRegion: QtObject {
        property int type: -1
        property int index: -1
    }

    // layout
    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 0
    z: -1

    // manage selected region
    function switchSelectedRegion(type, index) {
        currRegion.type = type;
        currRegion.index = index;
        regions.rectSelected = currRegion.index >= 0;

        matrix.setMask();
    }

    // matrix zone
    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        Rectangle {
            anchors.fill: parent
            color: Stylesheet.colors.black
        }

        GeneratorLattice {
            id: matrix

            generatorID: generatorIndex < 0 ? -1 : generatorModel.at(generatorIndex).id
            anchors.fill: parent

            // uniforms
            squareInPixels: ppc
            mask: Qt.vector4d(-1, -1, -1, -1)
            maskAlpha: 0.3

            function setMask(rect) {
                var element;

                // auto cancel if type is signed
                if (currRegion.type === -1) return mask = Qt.vector4d(-1, -1, -1, -1);

                // set selected to function arguments if applicable
                if (rect !== undefined) return mask = Qt.vector4d(rect.x, rect.y, rect.width, rect.height);

                // otherwise, retrieve automatically from global properties
                // TODO: clean up function calls so that this block can be removed, instead directly using arguments every time
                if (currRegion.type === 0) element = inputModel.at(currRegion.index);
                else if (currRegion.type === 1) element = outputModel.at(currRegion.index);
                mask = Qt.vector4d(element.rect.x, element.rect.y, element.rect.width, element.rect.height);
            }

            // TODO
            /*
            NumberAnimation on maskAlpha {
                duration: 300
                easing.type: Easing.InOutQuad
            }
            */
        }

        MouseArea {
            id: matrixMouseBg
            anchors.fill: parent
            onClicked: latticeView.switchSelectedRegion(-1, -1)
        }

        // I/O regions
        Item {
            id: regions
            visible: !(generatorIndex < 0)

            property int latticeWidth: generatorIndex < 0 ? 20 : generatorModel.at(generatorIndex).latticeWidth
            property int latticeHeight: generatorIndex < 0 ? 20 : generatorModel.at(generatorIndex).latticeHeight
            property bool rectSelected: false

            width: ppc * latticeWidth
            height: ppc * latticeHeight
            x: parent.width/2 - width/2
            y: parent.height/2 - height/2

            // inputs
            Repeater {
                model: inputModel
                Region {
                    type: 0
                    latticeWidth: regions.latticeWidth
                    latticeHeight: regions.latticeHeight
                    ppc: latticeView.ppc
                }
            }
            // outputs
            Repeater {
                model: outputModel
                Region {
                    type: 1
                    latticeWidth: regions.latticeWidth
                    latticeHeight: regions.latticeHeight
                    ppc: latticeView.ppc
                }
            }
        }

        // tooltip (shown when no generator is selected)
        Label {
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            width: 320

            visible: generatorIndex < 0

            text: qsTr("Select a generator from the left panel to visualize and edit it.")
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 14
            opacity: 0.4
        }
    }
}
