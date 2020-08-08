import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import ca.hexagram.xmodal.autonomx 1.0

import "qrc:/stylesheet"
import "../models"
import "../components/delegates"

ColumnLayout {
    id: latticeView

    property int genID: window.activeGeneratorIndex

    Layout.fillWidth: true
    Layout.fillHeight: true
    spacing: 0

    // header
    Header {}

    // main content
    RowLayout {
        id: mainContent

        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 0

        // props
        property int ppc: 20            // pixels per cell, ie. how wide a cell square is in pixels. this is animated within QML (scaled by the zoom factor)
        property QtObject currRegion: QtObject {
            property int type: -1
            property int index: -1
        }

        // manage selected region
        function switchSelectedRegion(type, index) {
            currRegion.type = type;
            currRegion.index = index;
            regions.rectSelected = currRegion.index >= 0;

            matrix.setMask();
        }

        // models
        InputModel { id: inputModel }
        OutputModel { id: outputModel }

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

                generatorID: 0
                anchors.fill: parent

                // uniforms
                squareInPixels: mainContent.ppc
                mask: Qt.vector4d(-1, -1, -1, -1)
                maskAlpha: 0.3

                function setMask(colX, colY, colW, colH) {
                    var element;

                    // auto cancel if type is signed
                    if (mainContent.currRegion.type === -1) return mask = Qt.vector4d(-1, -1, -1, -1);

                    // set selected to function arguments if applicable
                    if (colX !== undefined) return mask = Qt.vector4d(colX, colY, colW, colH);

                    // otherwise, retrieve automatically from global properties
                    // TODO: clean up function calls so that this block can be removed, instead directly using arguments every time
                    if (mainContent.currRegion.type === 0) element = inputModel.get(mainContent.currRegion.index);
                    else if (mainContent.currRegion.type === 1) element = outputModel.get(mainContent.currRegion.index);
                    mask = Qt.vector4d(element.colX, element.colY, element.colW, element.colH);
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
                onClicked: mainContent.switchSelectedRegion(-1, -1)
            }

            // I/O regions
            Item {
                id: regions
                visible: !(genID < 0)

                property int ppc: mainContent.ppc
                property int latticeWidth: genID < 0 ? 20 : generatorModel.at(genID).latticeWidth
                property int latticeHeight: genID < 0 ? 20 : generatorModel.at(genID).latticeHeight
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
                        ppc: regions.ppc
                    }
                }
                // outputs
                Repeater {
                    model: outputModel
                    Region {
                        type: 1
                        ppc: regions.ppc
                    }
                }
            }

            // tooltip (shown when no generator is selected)
            Label {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                width: 320

                visible: genID < 0

                text: qsTr("Please select a generator from the Generator View to edit its I/O zones.")
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                font {
                    family: Stylesheet.fonts.main
                    pixelSize: 14
                }
                opacity: 0.4
            }
        }

        // control zone
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
                anchors.fill: parent
                color: Stylesheet.colors.darkGrey
            }
        }
    }
}
