#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QtGui>

namespace img2chr
{
    class EditorWidget : public QWidget
    {
        Q_OBJECT
        public:
            EditorWidget();

        private slots:
            void browse();
            void toggledPadding(int state);
            void conversionChanged(const QString& text);

        private:
            QPushButton* imageBrowseButton;
            QRadioButton* compressionNone;
            QRadioButton* compressionRLE;
            QCheckBox* paddingOption;

            QLabel* imageFilenameLabel;
            QLabel* sourceColorsLabel;
            QLabel* destColorsLabel;
            QLabel* tileCountLabel;
            QLabel* imageLabel;
            QLabel* previewTileCountLabel;
            QLabel* previewImageLabel;

            QHBoxLayout* sourcePalette;
            QHBoxLayout* destPalette;
            QHBoxLayout* conversionFields;
            QList<int> conversions;

            QImage image;

            QRgb getPaletteColor(int i);
            void calculatePalette();
            void calculatePreview();
    };
}

#endif
