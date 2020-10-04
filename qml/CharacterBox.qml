/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQuick 2.13
import Scrite 1.0

Rectangle {
    property Character character

    width: 150
    height: 100
    color: "white"
    border.width: 1
    border.color: "black"
    radius: 6

    Row {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8
        clip: true

        Rectangle {
            width: parent.height
            height: parent.height
            border.width: 1
            border.color: primaryColors.borderColor

            Image {
                anchors.fill: parent
                anchors.margins: 1
                source: {
                    if(character.photos.length > 0)
                        return "file:///" + character.photos[0]
                    return "../icons/content/character_icon.png"
                }
                fillMode: Image.PreserveAspectFit
                mipmap: true; smooth: true
            }
        }

        Column {
            width: parent.width - parent.height - parent.spacing
            anchors.top: parent.top
            spacing: 4

            Text {
                text: character.name
                font.bold: true
                font.pointSize: app.idealFontPointSize
                font.capitalization: Font.AllUppercase
                width: parent.width
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pointSize: app.idealFontPointSize-3
                maximumLineCount: 3
                text: {
                    var fields = []
                    var addField = function(val, prefix) {
                        if(val && val !== "") {
                            if(prefix)
                                fields.push(prefix + ": " + val)
                            else
                                fields.push(val)
                        }
                    }
                    addField(character.designation)
                    addField(character.gender)
                    addField(character.age, "Age")
                    addField(character.height, "Height")
                    addField(character.weight, "Weight")
                    addField(character.aliases.join(", "), "Also known as")
                    return fields.join(", ")
                }
            }
        }
    }
}
