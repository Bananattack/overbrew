#include <QMessageBox>

#include "editorwidget.h"

namespace img2chr
{
    namespace
    {
        struct PaletteEntryComparator
        {
            PaletteEntryComparator(QVector<QRgb> colors)
                : colors(colors)
            {
            }

            bool operator()(int a, int b)
            {
                return qGray(colors[a]) < qGray(colors[b]);
            }

            QVector<QRgb> colors;
        };
    }

    QVector<int> EditorWidget::orderedPalette(const QImage& source)
    {
        QVector<QRgb> colors = source.colorTable();
        QVector<int> palette(colors.count());
        for(int i = 0, end = colors.count(); i != end; ++i)
        {
            palette[i] = i;
        }
        qStableSort(palette.begin(), palette.end(), PaletteEntryComparator(colors));
        return palette;
    }

    QImage EditorWidget::stripPadding(const QImage& source)
    {
        int columns(source.width() / (TILE_WIDTH + 1));
        int rows(source.height() / (TILE_HEIGHT + 1));

        QImage result(QSize(columns * TILE_WIDTH, rows * TILE_HEIGHT), QImage::Format_Indexed8);
        result.setColorTable(source.colorTable());

        for(int r = 0, rend = rows; r != rend; ++r)
        {
            for(int c = 0, cend = columns; c != cend; ++c)
            {
                for(int y = 0, yend = TILE_HEIGHT; y != yend; ++y)
                {
                    for(int x = 0, xend = TILE_WIDTH; x != xend; ++x)
                    {
                        result.setPixel(
                            c * TILE_WIDTH + x,
                            r * TILE_HEIGHT + y,
                            source.pixelIndex(
                                c * (TILE_WIDTH + 1) + 1 + x,
                                r * (TILE_HEIGHT + 1) + 1 + y
                            )
                        );
                    }
                }
            }
        }
        return result;
    }

    EditorWidget::EditorWidget()
        : padding(false)
    {
        QVBoxLayout* mainLayout(new QVBoxLayout());
        mainLayout->setAlignment(Qt::AlignTop);
        setLayout(mainLayout);

        if(QGroupBox* group = new QGroupBox(tr("Image")))
        {
            mainLayout->addWidget(group);

            QVBoxLayout* groupLayout(new QVBoxLayout());
            groupLayout->setAlignment(Qt::AlignCenter | Qt::AlignTop);
            group->setLayout(groupLayout);

            QHBoxLayout* browseLayout(new QHBoxLayout());
            browseLayout->setAlignment(Qt::AlignLeft);
            groupLayout->addLayout(browseLayout);

            imageFilenameLabel = new QLabel(tr("This tileset has no image yet."));
            browseLayout->addWidget(imageFilenameLabel);

            imageBrowseButton = new QPushButton(tr("&Import..."));
            imageBrowseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            imageBrowseButton->setAutoDefault(true);
            browseLayout->addWidget(imageBrowseButton);

            imageLabel = new QLabel();
            groupLayout->addWidget(imageLabel);
        }
        if(QGroupBox* group = new QGroupBox(tr("Palette")))
        {
            mainLayout->addWidget(group);

            QVBoxLayout* groupLayout(new QVBoxLayout());
            groupLayout->setAlignment(Qt::AlignCenter | Qt::AlignTop);
            group->setLayout(groupLayout);

            sourcePalette = new QHBoxLayout();
            sourcePalette->setAlignment(Qt::AlignCenter);
            sourcePalette->setSpacing(2);
            groupLayout->addLayout(sourcePalette);

            destPalette = new QHBoxLayout();
            destPalette->setAlignment(Qt::AlignCenter);
            destPalette->setSpacing(2);
            groupLayout->addLayout(destPalette);

            conversionFields = new QHBoxLayout();
            conversionFields->setAlignment(Qt::AlignCenter);
            conversionFields->setSpacing(2);
            groupLayout->addLayout(conversionFields);

            paletteHelpLabel = new QLabel(tr(
                "0 .. 3 are valid color indexes.<br>"
                "0 = transparent in sprites."
            ));
            paletteHelpLabel->hide();
            groupLayout->addWidget(paletteHelpLabel);

        }
        if(QHBoxLayout* rowLayout = new QHBoxLayout())
        {
            mainLayout->addLayout(rowLayout);
            if(QGroupBox* group = new QGroupBox(tr("Info")))
            {
                rowLayout->addWidget(group);

                QVBoxLayout* groupLayout(new QVBoxLayout());
                group->setLayout(groupLayout);

                sourceColorsLabel = new QLabel(tr("Source Colors: 0"));
                groupLayout->addWidget(sourceColorsLabel);

                tilesLabel = new QLabel(tr("Output Tiles: 0"));
                groupLayout->addWidget(tilesLabel);
            }
            if(QGroupBox* group = new QGroupBox(tr("Output Compression")))
            {
                rowLayout->addWidget(group);

                QVBoxLayout* groupLayout(new QVBoxLayout());
                group->setLayout(groupLayout);

                compressionNone = new QRadioButton(tr("None"));
                compressionNone->setChecked(true);
                groupLayout->addWidget(compressionNone);

                compressionRLE = new QRadioButton(tr("RLE (Run-Length Encoding)"));
                groupLayout->addWidget(compressionRLE);
            }
            if(QGroupBox* group = new QGroupBox(tr("Options")))
            {
                rowLayout->addWidget(group);

                QHBoxLayout* groupLayout(new QHBoxLayout());
                group->setLayout(groupLayout);

                paddingOption = new QCheckBox(tr("Remove Padding"));
                paddingOption->setChecked(true);
                padding = paddingOption->isChecked();
                groupLayout->addWidget(paddingOption);
            }
        }
        if(QGroupBox* group = new QGroupBox(tr("Preview")))
        {
            mainLayout->addWidget(group);

            QVBoxLayout* groupLayout(new QVBoxLayout());
            groupLayout->setAlignment(Qt::AlignCenter | Qt::AlignTop);
            group->setLayout(groupLayout);

            previewImageLabel = new QLabel(tr("No preview available."));
            previewImageLabel->setAlignment(Qt::AlignCenter);
            groupLayout->addWidget(previewImageLabel);

            previewHelpLabel = new QLabel(tr("(Make sure the preview you see has no padding around the tiles.)"));
            previewHelpLabel->setAlignment(Qt::AlignCenter);
            previewHelpLabel->hide();
            groupLayout->addWidget(previewHelpLabel);
        }

        connect(imageBrowseButton, SIGNAL(clicked()), this, SLOT(browse()));
        connect(paddingOption, SIGNAL(toggled(bool)), this, SLOT(toggledPadding(bool)));
    }

    void EditorWidget::browse()
    {
        QString filename(
            QFileDialog::getOpenFileName(
                this,
                tr("Import Image"),
                QString(),
                tr("Images (*.png *.gif *.bmp);;Tilesets (*.chr);;All Files (*.*)")
            )
        );
        if(!filename.isEmpty())
        {
            image = QImage(filename);
            if(!image.isNull())
            {
                image = image.convertToFormat(QImage::Format_RGB32, Qt::AvoidDither | Qt::ThresholdDither | Qt::ThresholdAlphaDither);
                image = image.convertToFormat(QImage::Format_Indexed8, Qt::AvoidDither | Qt::ThresholdDither | Qt::ThresholdAlphaDither);
                sourceColorsLabel->setText(tr("Source Colors: %1").arg(image.colorCount()));
                imageFilenameLabel->setText(tr("<b>%1</b>").arg(QFileInfo(filename).fileName()));
                imageLabel->setPixmap(QPixmap::fromImage(image));

                conversions.clear();
                while(QLayoutItem* child = sourcePalette->takeAt(0))
                {
                    delete child->widget();
                    delete child;
                }
                while(QLayoutItem* child = destPalette->takeAt(0))
                {
                    delete child->widget();
                    delete child;
                }
                while(QLayoutItem* child = conversionFields->takeAt(0))
                {
                    delete child->widget();
                    delete child;
                }
                for(int i = 0, end = image.colorCount(); i != end; ++i)
                {
                    if(QLabel* label = new QLabel())
                    {
                        sourcePalette->addWidget(label);

                        label->setMinimumWidth(16);
                        label->setMinimumHeight(16);
                        label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        label->setPalette(QPalette(QColor(image.color(i))));
                        label->setAutoFillBackground(true);
                    }
                    if(QLabel* label = new QLabel())
                    {
                        destPalette->addWidget(label);

                        label->setMinimumWidth(16);
                        label->setMinimumHeight(16);
                        label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        label->setAutoFillBackground(true);
                    }
                    if(QLineEdit* edit = new QLineEdit(tr("%1").arg(0)))
                    {
                        conversionFields->addWidget(edit);

                        edit->setValidator(new QIntValidator());
                        edit->setMinimumWidth(16);
                        edit->setMaximumWidth(16);
                        edit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

                        connect(edit, SIGNAL(textChanged(const QString&)), this, SLOT(conversionChanged(const QString&)));
                    }
                    conversions.append(0);
                }

                autoFillConversions();
                calculatePalette();
                calculatePreview();
            }
        }
    }

    void EditorWidget::toggledPadding(bool checked)
    {
        padding = checked;
        autoFillConversions();
        calculatePalette();
        calculatePreview();
    }

    void EditorWidget::conversionChanged(const QString& text)
    {
        calculatePalette();
        calculatePreview();
    }

    QRgb EditorWidget::getPaletteColor(int i)
    {
        switch(i)
        {
            case 0: return qRgb(0x50, 0x50, 0x50); break;
            case 1: return qRgb(0xA0, 0xA0, 0xA0); break;
            case 2: return qRgb(0xD0, 0xD0, 0xD0); break;
            case 3: return qRgb(0xFF, 0xFF, 0xFF); break;
            default: return qRgb(0xFF, 0x00, 0xFF); break;
        }
    }

    void EditorWidget::autoFillConversions()
    {
        // Remove padding color.
        QVector<int> palette(orderedPalette(image));
        if(padding && image.width() && image.height())
        {
            int colorIndex = image.pixelIndex(0, 0);
            QLineEdit* edit(qobject_cast<QLineEdit*>(conversionFields->itemAt(colorIndex)->widget()));
            edit->setText(tr("%1").arg(0));
            palette.remove(palette.indexOf(colorIndex));
            conversions[colorIndex] = 0;
        }

        // Remove 'transparent' color (fully-saturated colors).
        QVector<int>::iterator it(palette.begin());
        while(it != palette.end())
        {
            int colorIndex = *it;
            QRgb color = image.color(colorIndex);
            int r = qRed(color);
            int g = qGreen(color);
            int b = qBlue(color);
            if((r == 0x00 || r == 0xFF)
                && (g == 0x00 || g == 0xFF)
                && (b == 0x00 || b == 0xFF)
                && !(r == 0x00 && g == 0x00 && b == 0x00)
                && !(r == 0xFF && g == 0xFF && b == 0xFF))
            {
                it = palette.erase(it);
                QLineEdit* edit(qobject_cast<QLineEdit*>(conversionFields->itemAt(colorIndex)->widget()));
                edit->setText(tr("%1").arg(0));
                conversions[colorIndex] = 0;
            }
            else
            {
                ++it;
            }
        }

        for(int i = 0, end = palette.count(); i != end; ++i)
        {
            int conversion = i * 4 / end;
            int colorIndex = palette[i];

            QLineEdit* edit(qobject_cast<QLineEdit*>(conversionFields->itemAt(colorIndex)->widget()));
            edit->setText(tr("%1").arg(conversion));
            conversions[colorIndex] = conversion;
        }

        if(palette.count())
        {
            int colorIndex = palette[palette.count() - 1];
            QLineEdit* edit(qobject_cast<QLineEdit*>(conversionFields->itemAt(colorIndex)->widget()));
            edit->setText(tr("%1").arg(3));
            conversions[colorIndex] = 3;
        }
    }

    void EditorWidget::calculatePalette()
    {
        for(int i = 0, end = conversionFields->count(); i != end; ++i)
        {
            QLineEdit* edit(qobject_cast<QLineEdit*>(conversionFields->itemAt(i)->widget()));
            bool success;
            int index(edit->text().toInt(&success, 10));
            if(success)
            {
                if(index < 0)
                {
                    index = 0;
                    edit->setText(tr("%1").arg(index));
                }
                else if(index > 3)
                {
                    index = 3;
                    edit->setText(tr("%1").arg(index));
                }
                conversions[i] = index;
                QLabel* label(qobject_cast<QLabel*>(destPalette->itemAt(i)->widget()));
                label->setPalette(QPalette(QColor(getPaletteColor(index))));
            }
        }
    }

    void EditorWidget::calculatePreview()
    {
        if(!image.isNull())
        {
            if(padding)
            {
                preview = stripPadding(image);
            }
            else
            {
                preview = image.copy(0, 0, image.width() / TILE_WIDTH * TILE_WIDTH, image.height() / TILE_HEIGHT * TILE_HEIGHT);
            }
            for(int i = 0, end = conversions.count(); i != end; ++i)
            {
                preview.setColor(i, getPaletteColor(conversions[i]));
            }

            previewImageLabel->setText(tr(""));
            previewImageLabel->setPixmap(QPixmap::fromImage(preview));

            paletteHelpLabel->show();
            previewHelpLabel->show();

            int tiles = (preview.width() / TILE_WIDTH) * (preview.height() / TILE_HEIGHT);
            tilesLabel->setText(tr("Output Tiles: %1").arg(tiles));
        }
    }
}
