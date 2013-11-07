#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->geometry().width(),this->geometry().height());
    this->m_progressDialog = new ProgressDialog();
    this->m_progressDialog->setWindowModality(Qt::ApplicationModal);
    this->m_progressDialog->setWindowFlags(this->m_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);

    this->m_settings = new QSettings("MegaPirateNG", "FlashTool");

    connect(ui->btnSerialRefresh, SIGNAL(clicked()), SLOT(updateSerialPorts()));
    connect(ui->cmbPlatform, SIGNAL(currentIndexChanged(int)), SLOT(platformChanged(int)));
    connect(ui->btnFlash, SIGNAL(clicked()), SLOT(startFlash()));

    this->updateSerialPorts();
    this->updateConfigs();
}

void MainWindow::updateConfigs()
{
    connect(this->m_progressDialog, SIGNAL(downloadsFinished(DownloadsList)), this, SLOT(parseConfigs(DownloadsList)));
    this->m_progressDialog->setLabelText(tr("Updating available firmwares..."));
    this->m_progressDialog->show();
    this->m_progressDialog->startDownloads(Download(FLASHTOOL_PATH_URI));
}

void MainWindow::parseConfigs(DownloadsList downloads)
{
    Download download = downloads[0];

    if (!download.success) {
        QMessageBox::critical(this, tr("FlashTool"), tr("Failed to download firmware informations, try again later."));

        QFile::remove(download.tmpFile);

        QApplication::exit();
        return;
    }

    disconnect(this->m_progressDialog, SIGNAL(downloadsFinished(DownloadsList)), this, SLOT(parseConfigs(DownloadsList)));


    QString oldBoardType = this->m_settings->value("BoardType").toString();
    QString oldRCInput = this->m_settings->value("RCInput").toString();
    QString oldRCInputMapping = this->m_settings->value("RCInputMapping").toString();
    QString oldPlatform = this->m_settings->value("Platform").toString();
    QString oldGpsType = this->m_settings->value("GpsType").toString();
    QString oldGpsBaud = this->m_settings->value("GpsBaud").toString();

    int oldBoardTypeIndex = 0;
    int oldRCInputIndex = 0;
    int oldRCInputMappingIndex = 0;
    int oldPlatformIndex = 0;
    int oldGpsTypeIndex = 0;
    int oldGpsBaudIndex = 0;

    this->m_progressDialog->setLabelText(tr("Checking available firmwares..."));

    QFile *file = new QFile(download.tmpFile);
    file->open(QIODevice::ReadOnly | QIODevice::Text);

    QXmlStreamReader xml(file);

    while (!xml.atEnd()) {
        xml.readNext();

        //Settings
        if (xml.isStartElement() && (xml.name() == "settings")) {
            this->m_globalsettings.hexnamepattern = xml.attributes().value("hexnamepattern").toString().simplified();
            this->m_globalsettings.hexurl = xml.attributes().value("hexurl").toString().simplified();
        }

        //Boards
        if (xml.isStartElement() && (xml.name() == "boards")) {
            ui->cmbBoardType->clear();
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isStartElement() && (xml.name() == "board")) {
                    BoardType board;
                    board.name = xml.attributes().value("name").toString().simplified();
                    board.id = xml.attributes().value("id").toString().simplified();

                    if (board.id == oldBoardType) {
                        oldBoardTypeIndex = ui->cmbBoardType->count();
                    }

                    QVariant vBoard;
                    vBoard.setValue<BoardType>(board);
                    ui->cmbBoardType->addItem(board.name, vBoard);
                }
                if (xml.isEndElement() && (xml.name() == "boards")) {
                    break;
                }
            }
        }

        //RC Inputs
        if (xml.isStartElement() && (xml.name() == "rcinputs")) {
            ui->cmbRCType->clear();
            ui->cmbRCMapping->clear();
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isStartElement() && (xml.name() == "rcinput")) {
                    RCInput input;
                    input.name = xml.attributes().value("name").toString().simplified();
                    input.id = xml.attributes().value("id").toString().simplified();

                    if (input.id == oldRCInput) {
                        oldRCInputIndex = ui->cmbRCType->count();
                    }

                    QVariant vInput;
                    vInput.setValue<RCInput>(input);
                    ui->cmbRCType->addItem(input.name, vInput);
                }
                if (xml.isStartElement() && (xml.name() == "rcmapping")) {
                    RCInputMapping input;
                    input.name = xml.attributes().value("name").toString().simplified();
                    input.id = xml.attributes().value("id").toString().simplified();

                    if (input.id == oldRCInputMapping) {
                        oldRCInputMappingIndex = ui->cmbRCMapping->count();
                    }

                    QVariant vInput;
                    vInput.setValue<RCInputMapping>(input);
                    ui->cmbRCMapping->addItem(input.name, vInput);
                }
                if (xml.isEndElement() && (xml.name() == "rcinputs")) {
                    break;
                }
            }
        }

        //Platforms
        if (xml.isStartElement() && (xml.name() == "platforms")) {
            ui->cmbPlatform->clear();
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isStartElement() && (xml.name() == "platform")) {
                    Platform platform;
                    platform.name = xml.attributes().value("name").toString().simplified();
                    platform.id = xml.attributes().value("id").toString().simplified();
                    platform.image = xml.attributes().value("image").toString().simplified();
                    platform.version = xml.attributes().value("version").toString().simplified();

                    if (platform.id == oldPlatform) {
                        oldPlatformIndex = ui->cmbPlatform->count();
                    }

                    QVariant vPlatform;
                    vPlatform.setValue<Platform>(platform);
                    ui->cmbPlatform->addItem(platform.name, vPlatform);
                }
                if (xml.isEndElement() && (xml.name() == "platforms")) {
                    break;
                }
            }
        }

        //GPS
        if (xml.isStartElement() && (xml.name() == "gps")) {
            ui->cmbGpsType->clear();
            ui->cmbGpsBaud->clear();
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isStartElement() && (xml.name() == "gpstype")) {
                    GpsType gpstype;
                    gpstype.name = xml.attributes().value("name").toString().simplified();
                    gpstype.id = xml.attributes().value("id").toString().simplified();

                    if (gpstype.id == oldGpsType) {
                        oldGpsTypeIndex = ui->cmbGpsType->count();
                    }

                    QVariant vGpsType;
                    vGpsType.setValue<GpsType>(gpstype);
                    ui->cmbGpsType->addItem(gpstype.name, vGpsType);
                }
                if (xml.isStartElement() && (xml.name() == "gpsbaud")) {
                    GpsBaudrate gpsbaud;
                    gpsbaud.name = xml.attributes().value("name").toString().simplified();
                    gpsbaud.id = xml.attributes().value("id").toString().simplified();

                    if (gpsbaud.id == oldGpsBaud) {
                        oldGpsBaudIndex = ui->cmbGpsBaud->count();
                    }

                    QVariant vGpsBaud;
                    vGpsBaud.setValue<GpsBaudrate>(gpsbaud);
                    ui->cmbGpsBaud->addItem(gpsbaud.name, vGpsBaud);
                }
                if (xml.isEndElement() && (xml.name() == "gps")) {
                    break;
                }
            }
        }

        //Versions
        if (xml.isStartElement() && (xml.name() == "versions")) {
            this->m_versionList.clear();
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isStartElement() && (xml.name() == "version")) {
                    Version version;
                    version.number = xml.attributes().value("number").toString().simplified();
                    version.id = xml.attributes().value("id").toString().simplified();
                    version.platform = xml.attributes().value("platform").toString().simplified();
                    this->m_versionList<<version;
                }
                if (xml.isEndElement() && (xml.name() == "versions")) {
                    break;
                }
            }
        }
    }

    file->close();
    file->remove();

    //Now set old values if they still exists
    ui->cmbBoardType->setCurrentIndex(oldBoardTypeIndex);
    ui->cmbRCType->setCurrentIndex(oldRCInputIndex);
    ui->cmbRCMapping->setCurrentIndex(oldRCInputMappingIndex);
    ui->cmbPlatform->setCurrentIndex(oldPlatformIndex);
    ui->cmbGpsBaud->setCurrentIndex(oldGpsBaudIndex);
    ui->cmbGpsType->setCurrentIndex(oldGpsTypeIndex);
    this->platformChanged(ui->cmbPlatform->currentIndex());
    this->m_progressDialog->hide();
}

void MainWindow::updateSerialPorts()
{
    ui->cmbSerialPort->clear();
    ui->cmbSerialPort->setDisabled(false);
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts()) {
        if (!info.portName.isEmpty()) {
            ui->cmbSerialPort->addItem(info.portName);
        }
    }

    if (ui->cmbSerialPort->count() == 0)
    {
        ui->cmbSerialPort->setDisabled(true);
        ui->cmbSerialPort->addItem(tr("- no serial port found -"));
    }
}

void MainWindow::platformChanged(int index)
{
    QString oldVersion = this->m_settings->value("Version").toString();
    int oldVersionIndex = 0;

    Platform platform = ui->cmbPlatform->itemData(index).value<Platform>();
    ui->lblImage->setStyleSheet("background: transparent url(:/images/resources/" + platform.image + ") center 0 no-repeat;");
    ui->cmbVersion->clear();
    ui->cmbVersion->setDisabled(false);
    for (int i = 0; i < this->m_versionList.count(); i++)
    {
        Version version = this->m_versionList[i];
        if (version.platform == platform.version)
        {
            if (version.id == oldVersion) {
                oldVersionIndex = ui->cmbVersion->count();
            }
            QVariant vVersion;
            vVersion.setValue<Version>(version);
            ui->cmbVersion->addItem(version.number, vVersion);
        }
    }

    if (ui->cmbVersion->count() == 0)
    {
        ui->cmbVersion->setDisabled(true);
        ui->cmbVersion->addItem(tr("- no flashable version found -"));
    } else {
        ui->cmbVersion->setCurrentIndex(oldVersionIndex);
    }
}

void MainWindow::startFlash()
{
    if (!ui->cmbSerialPort->isEnabled())
    {
        QMessageBox::critical(this, tr("FlashTool"), tr("No serial port found, please make sure you connected your board via usb."));
        //return;
    }

    if (!ui->cmbVersion->isEnabled())
    {
        QMessageBox::critical(this, tr("FlashTool"), tr("No flashable version found."));
        return;
    }

    BoardType board = ui->cmbBoardType->itemData(ui->cmbBoardType->currentIndex()).value<BoardType>();
    RCInput rcinput = ui->cmbRCType->itemData(ui->cmbRCType->currentIndex()).value<RCInput>();
    RCInputMapping rcinputmapping = ui->cmbRCMapping->itemData(ui->cmbRCMapping->currentIndex()).value<RCInputMapping>();
    Platform platform = ui->cmbPlatform->itemData(ui->cmbPlatform->currentIndex()).value<Platform>();
    Version version = ui->cmbVersion->itemData(ui->cmbVersion->currentIndex()).value<Version>();
    GpsType gpstype = ui->cmbGpsType->itemData(ui->cmbGpsType->currentIndex()).value<GpsType>();
    GpsBaudrate gpsbaud = ui->cmbGpsBaud->itemData(ui->cmbGpsBaud->currentIndex()).value<GpsBaudrate>();

    connect(this->m_progressDialog, SIGNAL(downloadsFinished(DownloadsList)), this, SLOT(prepareSourceCode(DownloadsList)));
    this->m_progressDialog->setLabelText((tr("Downloading firmware %1 (%2) ...").arg(platform.name).arg(version.number)));
    this->m_progressDialog->show();

    QString hexname = this->m_globalsettings.hexnamepattern;
    QString url = this->m_globalsettings.hexurl;

    hexname.replace("%board%", board.id);
    hexname.replace("%rcinput%", rcinput.id);
    hexname.replace("%rcmapping%", rcinputmapping.id);
    hexname.replace("%platform%", platform.id);
    hexname.replace("%version%", version.id);
    hexname.replace("%gpstype%", gpstype.id);
    hexname.replace("%gpsbaud%", gpsbaud.id);

    this->m_progressDialog->startDownloads(Download(url + hexname));
}

void MainWindow::prepareSourceCode(DownloadsList downloads)
{
    disconnect(this->m_progressDialog, SIGNAL(downloadsFinished(DownloadsList)), this, SLOT(prepareSourceCode(DownloadsList)));

    if (!downloads[0].success) {
        QMessageBox::critical(this, tr("FlashTool"), tr("Failed to download firmware, try again later."));
    } else {
        //Flashing logic
    }

    //This is just until the flashing itself is finished
    for (int i = 0; i < downloads.count(); i++) {
        QFile::remove(downloads[i].tmpFile);
    }

    this->m_progressDialog->hide();
}

MainWindow::~MainWindow()
{
    BoardType board = ui->cmbBoardType->itemData(ui->cmbBoardType->currentIndex()).value<BoardType>();
    RCInput rcinput = ui->cmbRCType->itemData(ui->cmbRCType->currentIndex()).value<RCInput>();
    RCInputMapping rcinputmapping = ui->cmbRCMapping->itemData(ui->cmbRCMapping->currentIndex()).value<RCInputMapping>();
    Platform platform = ui->cmbPlatform->itemData(ui->cmbPlatform->currentIndex()).value<Platform>();
    Version version = ui->cmbVersion->itemData(ui->cmbVersion->currentIndex()).value<Version>();
    GpsType gpstype = ui->cmbGpsType->itemData(ui->cmbGpsType->currentIndex()).value<GpsType>();
    GpsBaudrate gpsbaud = ui->cmbGpsBaud->itemData(ui->cmbGpsBaud->currentIndex()).value<GpsBaudrate>();

    this->m_settings->setValue("BoardType", board.id);
    this->m_settings->setValue("RCInput", rcinput.id);
    this->m_settings->setValue("RCInputMapping", rcinputmapping.id);
    this->m_settings->setValue("Platform", platform.id);
    this->m_settings->setValue("Version", version.id);
    this->m_settings->setValue("GpsType", gpstype.id);
    this->m_settings->setValue("GpsBaud", gpsbaud.id);

    delete ui;
}
