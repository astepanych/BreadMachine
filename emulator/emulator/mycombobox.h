#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include <QComboBox>

class MyComboBox : public QComboBox
{
    Q_OBJECT
public:
    MyComboBox();
    void showPopup(){};

};

#endif // MYCOMBOBOX_H
