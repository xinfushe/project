/********************************************************************************
** Form generated from reading UI file 'detectdialog.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DETECTDIALOG_H
#define UI_DETECTDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QTextEdit>

QT_BEGIN_NAMESPACE

class Ui_DetectDialog
{
public:
    QGridLayout *gridLayout;
    QTextEdit *outputTextEdit;
    QLineEdit *sourceFileEdit;
    QDialogButtonBox *buttonBox;
    QPushButton *browseButton;
    QLabel *sourceFileLabel;
    QRadioButton *VideoRadioButton;
    QRadioButton *ImageRadioButton;
    QRadioButton *mosaicRadioButton;
    QRadioButton *squareRadioButton;
    QRadioButton *otherRadioButton;
    QButtonGroup *buttonGroup_2;
    QButtonGroup *buttonGroup;

    void setupUi(QDialog *DetectDialog)
    {
        if (DetectDialog->objectName().isEmpty())
            DetectDialog->setObjectName(QString::fromUtf8("DetectDialog"));
        DetectDialog->resize(324, 238);
        gridLayout = new QGridLayout(DetectDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        outputTextEdit = new QTextEdit(DetectDialog);
        outputTextEdit->setObjectName(QString::fromUtf8("outputTextEdit"));
        outputTextEdit->setReadOnly(true);

        gridLayout->addWidget(outputTextEdit, 4, 0, 1, 4);

        sourceFileEdit = new QLineEdit(DetectDialog);
        sourceFileEdit->setObjectName(QString::fromUtf8("sourceFileEdit"));

        gridLayout->addWidget(sourceFileEdit, 1, 1, 1, 2);

        buttonBox = new QDialogButtonBox(DetectDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 5, 1, 1, 3);

        browseButton = new QPushButton(DetectDialog);
        browseButton->setObjectName(QString::fromUtf8("browseButton"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(browseButton->sizePolicy().hasHeightForWidth());
        browseButton->setSizePolicy(sizePolicy);

        gridLayout->addWidget(browseButton, 1, 3, 1, 1);

        sourceFileLabel = new QLabel(DetectDialog);
        sourceFileLabel->setObjectName(QString::fromUtf8("sourceFileLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(sourceFileLabel->sizePolicy().hasHeightForWidth());
        sourceFileLabel->setSizePolicy(sizePolicy1);
        sourceFileLabel->setFrameShape(QFrame::NoFrame);
        sourceFileLabel->setFrameShadow(QFrame::Plain);

        gridLayout->addWidget(sourceFileLabel, 1, 0, 1, 1);

        VideoRadioButton = new QRadioButton(DetectDialog);
        buttonGroup_2 = new QButtonGroup(DetectDialog);
        buttonGroup_2->setObjectName(QString::fromUtf8("buttonGroup_2"));
        buttonGroup_2->addButton(VideoRadioButton);
        VideoRadioButton->setObjectName(QString::fromUtf8("VideoRadioButton"));

        gridLayout->addWidget(VideoRadioButton, 0, 0, 1, 1);

        ImageRadioButton = new QRadioButton(DetectDialog);
        buttonGroup_2->addButton(ImageRadioButton);
        ImageRadioButton->setObjectName(QString::fromUtf8("ImageRadioButton"));

        gridLayout->addWidget(ImageRadioButton, 0, 1, 1, 1);

        mosaicRadioButton = new QRadioButton(DetectDialog);
        buttonGroup = new QButtonGroup(DetectDialog);
        buttonGroup->setObjectName(QString::fromUtf8("buttonGroup"));
        buttonGroup->addButton(mosaicRadioButton);
        mosaicRadioButton->setObjectName(QString::fromUtf8("mosaicRadioButton"));

        gridLayout->addWidget(mosaicRadioButton, 3, 1, 1, 1);

        squareRadioButton = new QRadioButton(DetectDialog);
        buttonGroup->addButton(squareRadioButton);
        squareRadioButton->setObjectName(QString::fromUtf8("squareRadioButton"));

        gridLayout->addWidget(squareRadioButton, 3, 0, 1, 1);

        otherRadioButton = new QRadioButton(DetectDialog);
        buttonGroup->addButton(otherRadioButton);
        otherRadioButton->setObjectName(QString::fromUtf8("otherRadioButton"));

        gridLayout->addWidget(otherRadioButton, 3, 2, 1, 1);

#ifndef QT_NO_SHORTCUT
        sourceFileLabel->setBuddy(sourceFileEdit);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(sourceFileEdit, browseButton);
        QWidget::setTabOrder(browseButton, outputTextEdit);

        retranslateUi(DetectDialog);

        QMetaObject::connectSlotsByName(DetectDialog);
    } // setupUi

    void retranslateUi(QDialog *DetectDialog)
    {
        DetectDialog->setWindowTitle(QApplication::translate("DetectDialog", "Face Detection", 0, QApplication::UnicodeUTF8));
        browseButton->setText(QApplication::translate("DetectDialog", "&Browse", 0, QApplication::UnicodeUTF8));
        browseButton->setShortcut(QApplication::translate("DetectDialog", "Alt+B", 0, QApplication::UnicodeUTF8));
        sourceFileLabel->setText(QApplication::translate("DetectDialog", "&Source File:", 0, QApplication::UnicodeUTF8));
        VideoRadioButton->setText(QApplication::translate("DetectDialog", "&Video", 0, QApplication::UnicodeUTF8));
        ImageRadioButton->setText(QApplication::translate("DetectDialog", "Image", 0, QApplication::UnicodeUTF8));
        mosaicRadioButton->setText(QApplication::translate("DetectDialog", "&Mosaic", 0, QApplication::UnicodeUTF8));
        squareRadioButton->setText(QApplication::translate("DetectDialog", "&Square", 0, QApplication::UnicodeUTF8));
        otherRadioButton->setText(QApplication::translate("DetectDialog", "&Other", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DetectDialog: public Ui_DetectDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DETECTDIALOG_H
