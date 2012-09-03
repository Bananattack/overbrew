#include <QMessageBox>

#include "editorwidget.h"

namespace img2chr
{
    EditorWidget::EditorWidget()
    {
        QHBoxLayout* imageBrowseLayout = new QHBoxLayout();
        imageFilenameLabel = new QLabel(tr("This tileset has no image yet."));
        imageBrowseButton = new QPushButton(tr("&Import..."));
        imageBrowseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        imageBrowseButton->setAutoDefault(true);
        imageBrowseLayout->addWidget(imageFilenameLabel);
        imageBrowseLayout->addWidget(imageBrowseButton);
        imageBrowseLayout->setAlignment(Qt::AlignLeft);

        imageLabel = new QLabel();
        QVBoxLayout* imageGroupLayout = new QVBoxLayout();
        imageGroupLayout->addLayout(imageBrowseLayout);
        imageGroupLayout->addWidget(imageLabel);
        imageGroupLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        QGroupBox* imageGroup = new QGroupBox(tr("Image"));
        imageGroup->setLayout(imageGroupLayout);

        QVBoxLayout* paletteGroupLayout = new QVBoxLayout();
        sourceColorsLabel = new QLabel(tr("Source Colors: 0"));
        destColorsLabel = new QLabel(tr("Destination Colors: 4"));
        sourcePalette = new QHBoxLayout();
        sourcePalette->setAlignment(Qt::AlignLeft);
        sourcePalette->setSpacing(2);
        destPalette = new QHBoxLayout();
        destPalette->setAlignment(Qt::AlignLeft);
        destPalette->setSpacing(2);
        conversionFields = new QHBoxLayout();
        conversionFields->setAlignment(Qt::AlignLeft);
        conversionFields->setSpacing(2);
        paletteGroupLayout->addWidget(sourceColorsLabel);
        paletteGroupLayout->addWidget(destColorsLabel);
        paletteGroupLayout->addLayout(sourcePalette);
        paletteGroupLayout->addLayout(destPalette);
        paletteGroupLayout->addLayout(conversionFields);
        QGroupBox* paletteGroup = new QGroupBox(tr("Palette"));
        paletteGroup->setLayout(paletteGroupLayout);

        QVBoxLayout* compressionGroupLayout = new QVBoxLayout();
        compressionNone = new QRadioButton(tr("None"));
        compressionRLE = new QRadioButton(tr("RLE (Run-Length Encoding)"));
        compressionNone->setChecked(true);
        compressionGroupLayout->addWidget(compressionNone);
        compressionGroupLayout->addWidget(compressionRLE);
        QGroupBox* compressionGroup = new QGroupBox(tr("Output Compression"));
        compressionGroup->setLayout(compressionGroupLayout);

        QHBoxLayout* optionsGroupLayout = new QHBoxLayout();
        paddingOption = new QCheckBox(tr("Padding"));
        optionsGroupLayout->addWidget(paddingOption);
        QGroupBox* optionsGroup = new QGroupBox(tr("Options"));
        optionsGroup->setLayout(optionsGroupLayout);

        QVBoxLayout* previewGroupLayout = new QVBoxLayout();
        previewTileCountLabel = new QLabel(tr("Tiles: 0"));
        previewImageLabel = new QLabel();
        previewGroupLayout->addWidget(previewTileCountLabel);
        previewGroupLayout->addWidget(previewImageLabel);
        QGroupBox* previewGroup = new QGroupBox(tr("Preview"));
        previewGroup->setLayout(previewGroupLayout);

        QVBoxLayout* layout = new QVBoxLayout();
        layout->addWidget(imageGroup);
        layout->addWidget(paletteGroup);
        layout->addWidget(compressionGroup);
        layout->addWidget(optionsGroup);
        layout->addWidget(previewGroup);
        layout->setAlignment(Qt::AlignTop);
        setLayout(layout);

        connect(imageBrowseButton, SIGNAL(clicked()), this, SLOT(browse()));
        connect(paddingOption, SIGNAL(stateChanged(int)), this, SLOT(toggledPadding(int)));
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
                image = image.convertToFormat(QImage::Format_Indexed8, Qt::AvoidDither | Qt::ThresholdDither | Qt::ThresholdAlphaDither);
                sourceColorsLabel->setText(tr("Source Colors: %1").arg(image.colorCount()));
                imageFilenameLabel->setText(tr("<b>%1</b>").arg(QFileInfo(filename).fileName()));
                imageLabel->setPixmap(QPixmap::fromImage(image));

                conversions.clear();
                for(int i = 0; i < sourcePalette->count(); i++)
                {
                    sourcePalette->itemAt(i)->widget()->close();
                    destPalette->itemAt(i)->widget()->close();
                    conversionFields->itemAt(i)->widget()->close();
                }

                for(int i = 0; i < image.colorCount(); i++)
                {
                    QLabel* label = new QLabel();
                    label->setMinimumWidth(16);
                    label->setMinimumHeight(16);
                    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                    label->setPalette(QPalette(QColor(image.color(i))));
                    label->setAutoFillBackground(true);
                    sourcePalette->addWidget(label);

                    label = new QLabel();
                    label->setMinimumWidth(16);
                    label->setMinimumHeight(16);
                    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                    label->setAutoFillBackground(true);
                    destPalette->addWidget(label);
                    conversions.append(i % 4);

                    QLineEdit* edit = new QLineEdit(tr("%1").arg(i % 4));
                    edit->setMinimumWidth(16);
                    edit->setMaximumWidth(16);
                    edit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
                    conversionFields->addWidget(edit);
                    connect(edit, SIGNAL(textChanged(const QString&)), this, SLOT(conversionChanged(const QString&)));
                }

                previewTileCountLabel->setText(tr("Tiles: %1").arg((image.width() / 8) * (image.height() / 8)));

                calculatePalette();
                calculatePreview();
                resize(minimumSizeHint());
            }
        }
    }

    void EditorWidget::toggledPadding(int state)
    {
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

    void EditorWidget::calculatePalette()
    {
        for(int i = 0; i < conversionFields->count(); i++)
        {
            QLineEdit* edit(qobject_cast<QLineEdit*>(conversionFields->itemAt(i)->widget()));
            int index = edit->text().toInt();
            conversions[i] = index;

            QLabel* label(qobject_cast<QLabel*>(destPalette->itemAt(i)->widget()));
            label->setPalette(QPalette(QColor(getPaletteColor(index))));
        }
    }

    void EditorWidget::calculatePreview()
    {
        if(!image.isNull())
        {
            QImage preview = image.copy();
            for(int i = 0; i < conversions.size(); i++)
            {
                preview.setColor(i, getPaletteColor(conversions[i]));
            }
            previewImageLabel->setPixmap(QPixmap::fromImage(preview));
        }
    }
}
