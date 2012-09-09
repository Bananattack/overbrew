#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QtGui>

namespace chrbrew
{
    class EditorWidget : public QWidget
    {
        Q_OBJECT
        private:
            static const int TILE_WIDTH = 8;
            static const int TILE_HEIGHT = 8;

            static QVector<int> orderedPalette(const QImage& source);
            static QImage stripPadding(const QImage& source);

        public:
            EditorWidget();

        private slots:
            void browse();
            void toggledPadding(bool checked);
            void conversionChanged(const QString& text);

        public:
            bool readFile(const QString& filename);
            bool readCHR(const QString& filename);
            bool readImage(const QString& filename);
            bool writeCHR(const QString& filename);

        private:
            void setupImage(const QString& filename);
            QRgb getPaletteColor(int i);
            void autoFillConversions();
            void calculatePalette();
            void calculatePreview();

            QPushButton* imageBrowseButton;
            QRadioButton* compressionNone;
            QRadioButton* compressionRLE;
            QCheckBox* paddingOption;

            QLabel* imageFilenameLabel;
            QLabel* sourceColorsLabel;
            QLabel* tileCountLabel;
            QLabel* paletteHelpLabel;
            QLabel* tilesLabel;
            QLabel* previewHelpLabel;

            QLabel* imageLabel;
            QLabel* previewImageLabel;

            QHBoxLayout* sourcePalette;
            QHBoxLayout* destPalette;
            QHBoxLayout* conversionFields;
            QList<int> conversions;

            bool padding;
            QImage image;
            QImage preview;
            bool loading;
    };
}

#endif
