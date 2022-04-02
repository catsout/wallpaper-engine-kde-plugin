pragma Singleton
import QtQuick 2.0
import org.kde.kirigami 2.4 as Kirigami

Item {
    id: root_item
    property var textColor
    property var highlightColor
    property var highlightedTextColor
    property var backgroundColor
    property var activeBackgroundColor
    property var alternateBackgroundColor
    property var linkColor
    property var visitedLinkColor
    property var positiveTextColor
    property var positiveBackgroundColor
    property var neutralTextColor
    property var negativeTextColor
    property var disabledTextColor

    readonly property alias view: theme_view

    property bool _init: {
        setColor(name => Kirigami.Theme[name]);
        return true;
    }
    function setColor(get_color) {
        this.textColor                = get_color('textColor');
        this.highlightColor           = get_color('highlightColor');
        this.highlightedTextColor     = get_color('highlightedTextColor');
        this.backgroundColor          = get_color('backgroundColor');
        this.activeBackgroundColor    = get_color('activeBackgroundColor');
        this.alternateBackgroundColor = get_color('alternateBackgroundColor');
        this.linkColor                = get_color('linkColor');
        this.visitedLinkColor         = get_color('visitedLinkColor');
        this.positiveTextColor        = get_color('positiveTextColor');
        this.positiveBackgroundColor  = get_color('positiveBackgroundColor');
        this.neutralTextColor         = get_color('neutralTextColor');
        this.negativeTextColor        = get_color('negativeTextColor');
        this.disabledTextColor        = get_color('disabledTextColor');
    }

    Item {
        id: theme_view
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false

        property var textColor
        property var highlightColor
        property var highlightedTextColor
        property var backgroundColor
        property var activeBackgroundColor
        property var alternateBackgroundColor
        property var linkColor
        property var visitedLinkColor
        property var positiveTextColor
        property var positiveBackgroundColor
        property var neutralTextColor
        property var negativeTextColor
        property var disabledTextColor

        property bool _init: {
            const setColor = root_item.setColor.bind(this);
            setColor(name => Kirigami.Theme[name]);
            return true;
        }
    }
}
