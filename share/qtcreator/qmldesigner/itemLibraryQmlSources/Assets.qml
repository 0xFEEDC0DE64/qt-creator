/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuickDesignerTheme 1.0
import HelperWidgets 2.0
import StudioControls 1.0 as StudioControls
import StudioTheme 1.0 as StudioTheme

Item {
    DropArea {
        id: dropArea

        property var files // list of supported dropped files

        enabled: true
        anchors.fill: parent

        onEntered: {
            files = []
            for (var i = 0; i < drag.urls.length; ++i) {
                var url = drag.urls[i].toString();
                if (url.startsWith("file:///")) // remove file scheme (happens on Windows)
                    url = url.substr(8)
                var ext = url.slice(url.lastIndexOf('.') + 1).toLowerCase()
                if (rootView.supportedSuffixes().includes('*.' + ext))
                    files.push(url)
            }

            if (files.length === 0)
                drag.accepted = false;
        }

        onDropped: {
            if (files.length > 0)
                rootView.handleFilesDrop(files)
        }
    }

    ScrollView { // TODO: experiment using ListView instead of ScrollView + Column
        id: assetsView
        anchors.fill: parent

        Column {
            spacing: 2
            Repeater {
                model: assetsModel // context property
                delegate: dirSection
            }

            Component {
                id: dirSection

                Section {
                    width: assetsView.width -
                           (assetsView.verticalScrollBarVisible ? assetsView.verticalThickness : 0)
                    caption: dirName
                    sectionHeight: 30
                    sectionFontSize: 15
                    contentLevel: dirDepth
                    levelShift: 20
                    leftPadding: 0
                    hideHeader: dirDepth === 0
                    showLeftBorder: true
                    expanded: dirExpanded
                    visible: dirVisible
                    expandOnClick: false
                    onToggleExpand: {
                        dirExpanded = !dirExpanded
                    }

                    Column {
                        spacing: 5
                        leftPadding: 15

                        Repeater {
                            model: dirsModel
                            delegate: dirSection
                        }

                        Repeater {
                            model: filesModel
                            delegate: fileSection
                        }
                    }
                }
            }

            Component {
                id: fileSection

                Rectangle {
                    width: assetsView.width -
                           (assetsView.verticalScrollBarVisible ? assetsView.verticalThickness : 0)
                    height: img.height
                    color: mouseArea.containsMouse ? "#444444" : "transparent"

                    Row {
                        spacing: 5

                        Image {
                            id: img
                            asynchronous: true
                            width: 48
                            height: 48
                            source: "image://qmldesigner_assets/" + filePath
                        }

                        Text {
                            text: fileName
                            color: StudioTheme.Values.themeTextColor
                            font.pixelSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    readonly property string suffix: fileName.substr(-4)
                    readonly property bool isFont: suffix === ".ttf" || suffix === ".otf"

                    MouseArea {
                        id: mouseArea

                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton | Qt.RightButton

                        onExited: tooltipBackend.hideTooltip()
                        onCanceled: tooltipBackend.hideTooltip()
                        onPositionChanged: tooltipBackend.reposition()
                        onPressed: {
                            forceActiveFocus()
                            if (mouse.button === Qt.LeftButton)
                                rootView.startDragAsset(filePath)
                            else
                                print("TODO: impl context menu")
                        }

                        ToolTip {
                            visible: !isFont && mouseArea.containsMouse
                            text: filePath
                            delay: 1000
                        }

                        Timer {
                            interval: 1000
                            running: mouseArea.containsMouse
                            onTriggered: {
                                if (suffix === ".ttf" || suffix === ".otf") {
                                    tooltipBackend.name = fileName
                                    tooltipBackend.path = filePath
                                    tooltipBackend.showTooltip()
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
