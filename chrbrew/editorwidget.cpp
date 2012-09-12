#include <QMessageBox>

#include "editorwidget.h"

namespace chrbrew
{
    QVector<int> EditorWidget::orderedPalette(const QImage& source)
    {
        auto colors = source.colorTable();
        QVector<int> palette(colors.count());
        for(int i = 0, end = colors.count(); i != end; ++i)
        {
            palette[i] = i;
        }
        qStableSort(palette.begin(), palette.end(),
            [&](int a, int b)
            {
                return qGray(colors[a]) < qGray(colors[b]);
            }
        );
        return palette;
    }

    QImage EditorWidget::stripPadding(const QImage& source)
    {
        int columns(source.width() / (TILE_WIDTH + 1));
        int rows(source.height() / (TILE_HEIGHT + 1));

        QImage result(QSize(columns * TILE_WIDTH, rows * TILE_HEIGHT), QImage::Format_Indexed8);
        result.setColorTable(source.colorTable());

        for(int r = 0; r != rows; ++r)
        {
            for(int c = 0; c != columns; ++c)
            {
                for(int y = 0; y != TILE_HEIGHT; ++y)
                {
                    for(int x = 0; x != TILE_WIDTH; ++x)
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
    {
        auto mainLayout = new QVBoxLayout();
        mainLayout->setAlignment(Qt::AlignTop);
        setLayout(mainLayout);

        if(auto group = new QGroupBox(tr("Image")))
        {
            mainLayout->addWidget(group);

            auto groupLayout = new QVBoxLayout();
            groupLayout->setAlignment(Qt::AlignCenter | Qt::AlignTop);
            group->setLayout(groupLayout);

            auto browseLayout = new QHBoxLayout();
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
        if(auto group = new QGroupBox(tr("Palette")))
        {
            mainLayout->addWidget(group);

            auto groupLayout = new QVBoxLayout();
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
        if(auto rowLayout = new QHBoxLayout())
        {
            mainLayout->addLayout(rowLayout);
            if(auto group = new QGroupBox(tr("Info")))
            {
                rowLayout->addWidget(group);

                auto groupLayout = new QVBoxLayout();
                group->setLayout(groupLayout);

                sourceColorsLabel = new QLabel(tr("Source Colors: 0"));
                groupLayout->addWidget(sourceColorsLabel);

                tilesLabel = new QLabel(tr("Output Tiles: 0"));
                groupLayout->addWidget(tilesLabel);
            }
            if(auto group = new QGroupBox(tr("Output Compression")))
            {
                rowLayout->addWidget(group);

                auto groupLayout = new QVBoxLayout();
                group->setLayout(groupLayout);

                compressionNone = new QRadioButton(tr("None"));
                compressionNone->setChecked(true);
                groupLayout->addWidget(compressionNone);

                compressionRLE = new QRadioButton(tr("RLE (Run-Length Encoding)"));
                groupLayout->addWidget(compressionRLE);
            }
            if(auto group = new QGroupBox(tr("Options")))
            {
                rowLayout->addWidget(group);

                auto groupLayout = new QHBoxLayout();
                group->setLayout(groupLayout);

                paddingOption = new QCheckBox(tr("Remove Padding"));
                paddingOption->setChecked(true);
                padding = paddingOption->isChecked();
                groupLayout->addWidget(paddingOption);
            }
        }
        if(auto group = new QGroupBox(tr("Preview")))
        {
            mainLayout->addWidget(group);

            auto groupLayout = new QVBoxLayout();
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
        auto filename = QFileDialog::getOpenFileName(
            this,
            tr("Import Image"),
            QString(),
            tr(
                "Character Sets/Images (*.chr *.png *.gif *.bmp)"
                ";;Images (*.png *.gif *.bmp)"
                ";;Character Sets (*.chr)"
                ";;All Files (*.*)"
            )
        );
        if(!filename.isEmpty())
        {
            readFile(filename);
        }
    }

    void EditorWidget::toggledPadding(bool checked)
    {
        if(!loading)
        {
            padding = checked;
            autoFillConversions();
            calculatePalette();
            calculatePreview();
        }
    }

    void EditorWidget::conversionChanged(const QString& text)
    {
        calculatePalette();
        calculatePreview();
    }

    bool EditorWidget::readFile(const QString& filename)
    {
        auto suffix = QFileInfo(filename).suffix();
        bool success = false;

        loading = true;

        if(suffix == "chr")
        {
            success = readCHR(filename);
            if(success)
            {
                setupImage(filename);
            }
        }
        else if(suffix == "png" || suffix == "gif" || suffix == "bmp")
        {
            success = readImage(filename);
        }
        else
        {
            QMessageBox::critical(this->parentWidget(), tr("Import Failed"), tr("'%1' has unrecognized file extension.").arg(filename));
        }        

        if(success)
        {
            setupImage(filename);
        }
        loading = false;

        return success;
    }

    bool EditorWidget::readCHR(const QString &filename)
    {
        QFile file(filename);
        if(!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this->parentWidget(), tr("Import Failed"), tr("'%1' could not be imported as a CHR.").arg(filename));
            return false;
        }

        int tiles = file.size() / 16;
        if(tiles == 0)
        {
            QMessageBox::critical(this->parentWidget(), tr("Import Failed"), tr("'%1' has no tiles! (File size is %2 byte(s))").arg(filename).arg(file.size()));
            return false;
        }

        // Try to fit this in a nice rectangular region, so it's easier to view.
        int columns = 0;
        int rows = 0;
        for(int i = 16; i != 1; i /= 2)
        {
            if(tiles % i == 0)
            {
                if(i >= 8)
                {
                    columns = i;
                    rows = tiles / i;
                }
                else
                {
                    rows = i;
                    columns = tiles / i;
                }
                break;
            }
        }
        if(columns == 0)
        {
            columns = tiles;
            rows = 1;
        }

        qDebug() << "tiles " << tiles << " rows " << rows << " columns " << columns;

        image = QImage(columns * 8, rows * 8, QImage::Format_Indexed8);
        image.setColorCount(4);
        for(int i = 0; i != 4; ++i)
        {
            image.setColor(i, getPaletteColor(i));
        }
        image.fill(0);

        QByteArray bytes(file.read(file.size()));
        for(int r = 0; r != rows; ++r)
        {
            for(int c = 0; c != columns; ++c)
            {
                for(int j = 0; j != 8; ++j)
                {
                    int index = ((r * columns + c) * 8 + j) * 2;
                    unsigned char low = bytes[index];
                    unsigned char high = bytes[index + 1];
                    for(int i = 0; i != 8; ++i)
                    {
                        image.setPixel(
                            c * 8 + i,
                            r * 8 + j,
                            ((high & (1 << (7 - i))) ? 2 : 0) | ((low & (1 << (7 - i))) ? 1 : 0)
                        );
                    }
                }
            }
        }

        padding = false;
        paddingOption->setChecked(false);

        return true;
    }

    bool EditorWidget::readImage(const QString &filename)
    {
        image = QImage(filename);
        if(!image.isNull())
        {
            image = image.convertToFormat(QImage::Format_RGB32, Qt::AvoidDither | Qt::ThresholdDither | Qt::ThresholdAlphaDither);
            image = image.convertToFormat(QImage::Format_Indexed8, Qt::AvoidDither | Qt::ThresholdDither | Qt::ThresholdAlphaDither);
            return true;
        }
        QMessageBox::critical(this->parentWidget(), tr("Import Failed"), tr("'%1' could not be imported as an image.").arg(filename));
        return false;
    }

    bool EditorWidget::writeCHR(const QString& filename)
    {
        QFile file(filename);
        if(!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(this->parentWidget(), tr("Save Failed"), tr("Failed to open '%1' for writing").arg(filename));
            return false;
        }

        if(compressionRLE->isChecked())
        {
            QMessageBox::warning(this->parentWidget(), tr("Notice"), tr("RLE isn't implemented yet. Saving without compression."));
        }

        QByteArray bytes;
        for(int y = 0, h = preview.height(); y != h; y += 8)
        {
            for(int x = 0, w = preview.width(); x != w; x += 8)
            {
                for(int j = 0; j != 8; ++j)
                {
                    unsigned char low = 0;
                    unsigned char high = 0;
                    for(int i = 0; i != 8; ++i)
                    {
                        unsigned int color = conversions[preview.pixelIndex(x + i, y + j)];
                        low = (low << 1) | (color & 0x1);
                        high = (high << 1) | ((color & 0x2) >> 1);
                    }
                    bytes.append(low);
                    bytes.append(high);
                }
            }
        }
        file.write(bytes);
        file.close();
        return true;
    }

    void EditorWidget::setupImage(const QString& filename)
    {
        sourceColorsLabel->setText(tr("Source Colors: %1").arg(image.colorCount()));
        imageFilenameLabel->setText(tr("<b>%1</b>").arg(QFileInfo(filename).fileName()));
        imageLabel->setPixmap(QPixmap::fromImage(image));

        conversions.clear();
        while(auto child = sourcePalette->takeAt(0))
        {
            delete child->widget();
            delete child;
        }
        while(auto child = destPalette->takeAt(0))
        {
            delete child->widget();
            delete child;
        }
        while(auto child = conversionFields->takeAt(0))
        {
            delete child->widget();
            delete child;
        }
        for(int i = 0, end = image.colorCount(); i != end; ++i)
        {
            if(auto label = new QLabel())
            {
                sourcePalette->addWidget(label);

                label->setMinimumWidth(16);
                label->setMinimumHeight(16);
                label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                label->setPalette(QPalette(QColor(image.color(i))));
                label->setAutoFillBackground(true);
            }
            if(auto label = new QLabel())
            {
                destPalette->addWidget(label);

                label->setMinimumWidth(16);
                label->setMinimumHeight(16);
                label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                label->setAutoFillBackground(true);
            }
            if(auto edit = new QLineEdit(tr("%1").arg(0)))
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
        auto palette = orderedPalette(image);
        if(padding && image.width() && image.height())
        {
            int colorIndex = image.pixelIndex(0, 0);
            auto edit = qobject_cast<QLineEdit*>(conversionFields->itemAt(colorIndex)->widget());
            edit->setText(tr("%1").arg(0));
            palette.remove(palette.indexOf(colorIndex));
            conversions[colorIndex] = 0;
        }

        // Remove 'transparent' color (fully-saturated colors).
        auto it = palette.begin();
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
                auto edit = qobject_cast<QLineEdit*>(conversionFields->itemAt(colorIndex)->widget());
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

            auto edit = qobject_cast<QLineEdit*>(conversionFields->itemAt(colorIndex)->widget());
            edit->setText(tr("%1").arg(conversion));
            conversions[colorIndex] = conversion;
        }

        if(palette.count())
        {
            int colorIndex = palette[palette.count() - 1];
            auto edit = qobject_cast<QLineEdit*>(conversionFields->itemAt(colorIndex)->widget());
            edit->setText(tr("%1").arg(3));
            conversions[colorIndex] = 3;
        }
    }

    void EditorWidget::calculatePalette()
    {
        for(int i = 0, end = conversionFields->count(); i != end; ++i)
        {
            auto edit = qobject_cast<QLineEdit*>(conversionFields->itemAt(i)->widget());
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
