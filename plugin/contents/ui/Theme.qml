pragma Singleton
import QtQuick 2.0
import org.kde.kirigami 2.4 as Kirigami

QtObject {
    property var textColor
    property var highlightColor
    property var highlightedTextColor
    property var backgroundColor
    property var activeBackgroundColor
    property var linkColor
    property var visitedLinkColor
    property var positiveTextColor
    property var neutralTextColor
    property var negativeTextColor
    property var disabledTextColor

    property bool _init: {
        textColor             = Kirigami.Theme.textColor
        highlightColor        = Kirigami.Theme.highlightColor
        highlightedTextColor  = Kirigami.Theme.highlightedTextColor
        backgroundColor       = Kirigami.Theme.backgroundColor
        activeBackgroundColor = Kirigami.Theme.activeBackgroundColor
        linkColor             = Kirigami.Theme.linkColor
        visitedLinkColor      = Kirigami.Theme.visitedLinkColor
        positiveTextColor     = Kirigami.Theme.positiveTextColor
        neutralTextColor      = Kirigami.Theme.neutralTextColor
        negativeTextColor     = Kirigami.Theme.negativeTextColor
        disabledTextColor     = Kirigami.Theme.disabledTextColor

        return true;
    }
}
