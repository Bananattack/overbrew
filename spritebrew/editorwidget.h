#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QtGui>

namespace spritebrew
{
    class EditorWidget : public QWidget
    {
        Q_OBJECT
        private:
            static const int TILE_WIDTH = 8;
            static const int TILE_HEIGHT = 8;

        public:
            EditorWidget();

        private slots:
            void browse();
        public:
            bool readCHR(const QString& filename);

        private:
            void setupImage(const QString& filename);
            QRgb getPaletteColor(int i);

            QPushButton* imageBrowseButton;
            QLabel* imageFilenameLabel;
            QLabel* imageLabel;

            QImage image;
    };
}

#endif
