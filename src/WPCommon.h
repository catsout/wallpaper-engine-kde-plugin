#pragma once
#include <QObject>
#include <QQuickItem>

class WPCommon : public QObject
{
    Q_OBJECT

public:
    explicit WPCommon(QObject *parent = nullptr) : QObject(parent) {}
};
