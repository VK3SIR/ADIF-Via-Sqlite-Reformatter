#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "adifworker.h"
#include <QStatusBar>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QTableView>
#include <QDebug>
#include <QString>
#include <QStyleFactory>
#include <QLabel>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow) {

    ui->setupUi(this);

    setupUI();
}

MainWindow::~MainWindow() {

    if (db.isValid() && db.isOpen()) { // Check if the database instance is valid and open
        db.close();
        qDebug().noquote() << "Destructor: Database closed.";
    }
    delete ui;
}

void MainWindow::setupUI() {
    statusLabel = new QLabel();
    statusBar()->addPermanentWidget(statusLabel);

    // QApplication::setStyle(QStyleFactory::create("WindowsVista")); // Too Aggressive
    ui->progressBar->setStyle(QStyleFactory::create("WindowsVista"));

    statusBar()->addPermanentWidget(ui->progressBar, 0); // 0 is the stretch factor
    ui->progressBar->setFormat("%p%");
    ui->progressBar->setAlignment(Qt::AlignCenter);

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
    ui->progressBar->hide();

    statusBar()->addPermanentWidget(ui->progressBar, 0); // 0 is the stretch factor

    ui->statusbar->showMessage(tr("Ready"));

    // ui->actionFileReadDb->setEnabled(false);
    // ui->actionFileWriteDb->setEnabled(false);
    // ui->actionFileOpenDb->setEnabled(true);
    // ui->actionFileDisplayDb->setEnabled(false);
    // ui->actionFileCloseDb->setEnabled(false);
    setFileMenuOptions(0,0,1,0,0);

    ui->view->setEnabled(false);

    qApp->processEvents(); // prevent UI freeze
}

void MainWindow::setFileMenuOptions(bool frad, bool frda, bool fod, bool fdd, bool fcd) {
    ui->actionFileReadDb->setEnabled(frad);
    ui->actionFileWriteDb->setEnabled(frda);
    ui->actionFileOpenDb->setEnabled(fod);
    ui->actionFileDisplayDb->setEnabled(fdd);
    ui->actionFileCloseDb->setEnabled(fcd);
}

bool MainWindow::checkTableExists(QString tableName) {
    if (!db.open()) {
        qWarning() << "Database is not open!";
        return false;
    }

    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=:table");
    query.bindValue(":table", dbTable);

    if (!query.exec()) {
        qWarning() << "Failed to check table existence:" << query.lastError().text();
        return false;
    }

    return query.next(); // true if table exists
}

void MainWindow::setupDatabase() {
    if (!db.open()) {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(dbFile);
        db.open();
    }

    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", db.lastError().text());
        qDebug().noquote() << "Error deleting database file:" << " - " << db.lastError().text();
        exit(1);
    }    
}

// createTable() MUST be performed direcly after a setupDatabase()
// --> If the database is NOT opened in setupDatabase() all will fail !
int MainWindow::createTable() {

    int retval = 0;

    QSqlQuery query(db);

    QStringList upperCaseFields;
    for (const QString &field : dbFields) {
        upperCaseFields << field.toUpper() + " TEXT";
    }

    QStringList upperCasePrimaryKey;
    for (const QString &field : primaryKey) {
        upperCasePrimaryKey << field.toUpper() + " ";
    }

    //QString createTableSQL = QString("CREATE TABLE IF NOT EXISTS log (%1, PRIMARY KEY(%2))")
    //                             .arg(upperCaseFields.join(", "),upperCasePrimaryKey.join(", "));

    QString createTableSQL = QString("CREATE TABLE IF NOT EXISTS log (%1)")
                                 .arg(upperCaseFields.join(", "));

    if (query.exec(createTableSQL)) {
        QString msg = "Database 'dB' and Table 'log' created.";
        ui->statusbar->showMessage(msg);
        qDebug().noquote() << msg;
    } else {
        qDebug().noquote() << "Error creating table:" << query.lastError().text();
    }

    return retval;
}

int MainWindow::loadExistingDatabase() {

    int retval = 0;

    ui->statusbar->showMessage(tr("Opening existing database"));
    qApp->processEvents(); // prevent UI freeze

    if (QFile::exists(dbFile)) {
        qDebug().noquote() << "File exists:" << dbFile;
        if (!db.open()) {
            db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName(dbFile);
            db.open();

            ui->statusbar->showMessage(tr("Opened existing database"));
            qApp->processEvents(); // prevent UI freeze
        } else {
            ui->statusbar->showMessage(tr("Database already open"));
            qApp->processEvents(); // prevent UI freeze
        }
    } else {
        qDebug().noquote() << "Database does not exist:" << dbFile;
        ui->statusbar->showMessage(tr("Database does not exist"));
        qApp->processEvents(); // prevent UI freeze
    }

    if (db.isOpen()) {
        qDebug().noquote() << "Opening Database";
        // ui->actionFileReadDb->setEnabled(false);
        // ui->actionFileWriteDb->setEnabled(true);
        // ui->actionFileOpenDb->setEnabled(false);
        // ui->actionFileDisplayDb->setEnabled(true);
        // ui->actionFileCloseDb->setEnabled(true);
        setFileMenuOptions(0,1,0,1,1);
        //ui->view->setEnabled(false);

        retval = 1;
    } else {
        qDebug().noquote() << "Error in Opening Database";
        // ui->actionFileReadDb->setEnabled(false);
        // ui->actionFileWriteDb->setEnabled(true);
        // ui->actionFileOpenDb->setEnabled(false);
        // ui->actionFileDisplayDb->setEnabled(true);
        // ui->actionFileCloseDb->setEnabled(true);
        setFileMenuOptions(0,0,1,0,0);
        retval = 0;
    }

    return retval;
}

void MainWindow::loadAdifFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open ADIF File", "", "ADIF Files (*.adi *.adif);;All Files (*)");
    if (filePath.isEmpty()) return;

    ui->progressBar->show();
    qApp->processEvents(); // prevent UI freeze

    if (!db.isOpen()) {
        setupDatabase();
    }

    if (db.open()) {
        if (checkTableExists("log")) {
            qDebug().noquote() << "Table 'log' exists.";
        } else {
            qDebug().noquote() << "Table 'log' does not exist. Sending to 'log' table create.";
            createTable();
        }
    } else {
        QMessageBox::information(this, "Error", db.lastError().text());
        qWarning() << "Cannot create table as failed to open db:" << db.lastError().text();
        return;
    }

    ui->statusbar->showMessage(tr("Parsing ADIF to memory. Please Wait"));
    qApp->processEvents(); // prevent UI freeze

    // Moving this section of code to a threaded mode prevents UI freezing !
    // Based on https://chatgpt.com/c/68818440-b054-800b-b56f-513907fdce0d

    QThread *thread = new QThread;
    AdifWorker *worker = new AdifWorker;
    worker->moveToThread(thread);

    connect(thread, &QThread::started, [worker, filePath]() {
        worker->processFile(filePath);
    });

    connect(worker, &AdifWorker::finished, this, [=](const QList<QMap<QString, QString>> &records) {
        thread->quit();
        thread->wait();
        worker->deleteLater();
        thread->deleteLater();

        qDebug().noquote() << "Loaded" << records.size() << "records.";

        QSqlQuery query;
        db.transaction();

        QList<QString> fieldUpper;
        for (const QString& field : dbFields)
            fieldUpper.append(field.toUpper());

        QString fieldList = fieldUpper.join(", ");
        QString placeholders = QString("?, ").repeated(fieldUpper.size());
        placeholders.chop(2); // Remove the last ", "

        ui->statusbar->showMessage(tr("Writing ADIF to SQLITE. Please Wait"));
        ui->statusbar->repaint();

        int counter = 1;
        int subCount = 1;

        for (const auto& record : records) {
            query.prepare(QString("INSERT INTO log (%1) VALUES (%2)").arg(fieldList, placeholders));
            for (const auto& field : dbFields)
                query.addBindValue(record.value(field));
            query.exec();

            if ((counter % 100) == 0) {
                ui->progressBar->setValue(subCount);
                qApp->processEvents(); // prevent UI freeze
                subCount++;
            }
            counter++;
        }

        db.commit();

        ui->statusbar->showMessage(QString::number(records.size()) + " records loaded");
        qApp->processEvents(); // prevent UI freeze

        QMessageBox::information(this, "Success", QString("Inserted %1 records into database.").arg(records.size()));
        ui->progressBar->hide();
    });

    connect(worker, &AdifWorker::error, this, [=](const QString &msg) {
        qWarning() << "ADIF load error:" << msg;
        QMessageBox::critical(this, "Error", "ADIF load error: " + msg);
    });

    thread->start();
}

void MainWindow::setupModelAndView() {
    recordCount = 0;
    QString countStr;

    ui->view->setEnabled(true);

    QStringList tables = db.tables();
    if ((db.isOpen()) && (tables.contains("log", Qt::CaseInsensitive))) {

        model = new QSqlQueryModel(this);

        // You can write the query directly as a string
        // Why R"?".." See: https://chatgpt.com/c/688614a4-15b0-800b-9cec-cad8b9706225
        QString sql = R"(
            SELECT * FROM log
            ORDER BY qso_date ASC, time_on ASC
        )";
        model->setQuery(sql, db);

        // Loop needed as an incorrect/low number of records possibly returned
        while (model->canFetchMore()) {
            model->fetchMore();
        }
        recordCount = model->rowCount();

        countStr = QString("Records read: %1").arg(recordCount);

        qDebug().noquote() << countStr;
        ui->statusbar->showMessage(countStr);
        QApplication::processEvents();

        // Optionally, check for errors
        if (model->lastError().isValid()) {
            qWarning() << "Query Error:" << model->lastError().text();
        }

        // Adjust data in model collection to set header labels in CAPITALS if not already
        for (int i = 0; i < model->columnCount(); ++i) {
            QString header = model->headerData(i, Qt::Horizontal).toString().toUpper();
            model->setHeaderData(i, Qt::Horizontal, header);
        }

        QHeaderView *header = ui->view->horizontalHeader();
        header->setDefaultAlignment(Qt::AlignLeft);

        ui->view->setModel(model);
        ui->view->setSelectionBehavior(QTableView::SelectRows);
        ui->view->setSelectionMode(QTableView::SingleSelection);
        ui->view->setAlternatingRowColors(true);
        ui->view->setSortingEnabled(true);
        ui->view->resizeColumnsToContents();
    } else {
        ui->statusbar->showMessage("No database loaded.");
        ui->statusbar->repaint();
    }
}

void MainWindow::on_actionFileReadDb_triggered()
{
    loadAdifFile();

    // ui->actionFileReadDb->setEnabled(false);
    // ui->actionFileWriteDb->setEnabled(true);
    // ui->actionFileOpenDb->setEnabled(false);
    // ui->actionFileDisplayDb->setEnabled(true);
    // ui->actionFileCloseDb->setEnabled(true);
    setFileMenuOptions(0,1,0,1,1);

    ui->view->setEnabled(false);
}

void MainWindow::on_actionFileCloseDb_triggered()
{
    if (db.open()) {
        // Perform database operations...
        db.close(); // Close the database connection
    }

    ui->view->setEnabled(false);

    QString msg;

    if (QFile::exists(dbFile)) {
        if (QFile::remove(dbFile)) {
            msg = "Database file " + dbFile + " deleted";
            ui->statusbar->showMessage(msg);
            qDebug().noquote() << msg;
            QApplication::processEvents();
        } else {
            msg = "Error deleting database: " + db.lastError().text();
            QMessageBox::information(this, "Error", msg);
            qDebug().noquote() << msg;
        }
    } else {
        msg = "Database file " + dbFile + " already deleted";
    }

    ui->statusbar->showMessage(msg);
    qDebug().noquote() << msg;
    QApplication::processEvents();

    // If all ok

    recordCount=0;
    ui->progressBar->setValue(0);
    ui->actionFileReadDb->setEnabled(true);
    ui->statusbar->showMessage(tr("Database Empty/Deleted."));
    ui->view->setModel(NULL);
    QApplication::processEvents();

    // ui->actionFileReadDb->setEnabled(false);
    // ui->actionFileWriteDb->setEnabled(false);
    // ui->actionFileOpenDb->setEnabled(true);
    // ui->actionFileDisplayDb->setEnabled(false);
    // ui->actionFileCloseDb->setEnabled(false);
    setFileMenuOptions(0,0,1,0,0);
}

void MainWindow::on_actionFileWriteDb_triggered()
{
    QString msg;
    QString filename = QFileDialog::getSaveFileName(this, "Save ADIF", "", "*.adi");
    if (filename.isEmpty()) {
        msg = "Empty Filename: Cannot write ADIF file.";
        QMessageBox::critical(this, "Filename Error", msg + " Aborting Operation.");
        qDebug().noquote() << msg;
        return;
    }

    ui->progressBar->show();

    QSqlQuery query(QString("SELECT %1 FROM log").arg(dbFields.join(", ")));
    if (!query.isActive()) {
        msg = "Query Error: " + query.lastError().text();
        QMessageBox::critical(this, "Query Error", msg + " Aborting Operation.");
        qDebug().noquote() << msg;
        return;
    }

    msg = "Writing Records to " + filename;
    qDebug().noquote() << msg;
    ui->statusbar->showMessage(msg);

    QList<QString> records;
    while (query.next()) {
        QString record;
        for (int i = 0; i < dbFields.size(); ++i) {
            QString value = query.value(i).toString();
            record += QString("<%1:%2>%3 ").arg(dbFields[i]).arg(value.length()).arg(value);
        }
        record += "<EOR>";
        records.append(record);
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        msg = "Cannot write ADIF file.";
        QMessageBox::critical(this, "File Error", msg + " Aborting Operation." );
        qDebug().noquote() << msg;
        return;
    }

    QTextStream out(&file);

    // File Header
    out << "Qt6-cmake-based ADIF to SQL Demo Program. \n";
    out << "Demonstrates OpenAI(tm) generated & refined code \n";
    out << "By Steve VK3VM - July/August 2025 \n";
    out << "<ADIF_VER:5>1.0.0 \n";
    out << "<PROGRAMID:12>ADIFtoSQLITE \n";
    out << "<PROGRAMVERSION:3>0.1 \n";
    out << "<EOH> \n\n";

    ui->progressBar->setMaximum(records.size());
    ui->progressBar->setValue(0);

    for (int i = 0; i < records.size(); ++i) {
        out << records[i] << "\n";
        ui->progressBar->setValue(i + 1);
        qApp->processEvents(); // prevent UI freeze
    }

    file.close();

    ui->progressBar->hide();
    ui->statusbar->showMessage(QString::number(records.size()) + " records loaded");
    QApplication::processEvents();

    QMessageBox::information(this, "Success", QString("Inserted %1 records into database.").arg(records.size()));
    msg="";

    // ui->progressBar->hide();

    // ui->actionFileReadDb->setEnabled(false);
    // ui->actionFileWriteDb->setEnabled(true);
    // ui->actionFileOpenDb->setEnabled(true);
    // ui->actionFileDisplayDb->setEnabled(true);
    // ui->actionFileCloseDb->setEnabled(true);
    setFileMenuOptions(0,1,1,1,1);
}

void MainWindow::on_actionFileDisplayDb_triggered()
{
    ui->view->setEnabled(true);

    setupDatabase();

    // Set the QSQLITE_OPEN_READONLY option
    db.setConnectOptions("QSQLITE_OPEN_READONLY");

    setupModelAndView();

    // ui->actionFileReadDb->setEnabled(false);
    // ui->actionFileWriteDb->setEnabled(true);
    // ui->actionFileOpenDb->setEnabled(false);
    // ui->actionFileDisplayDb->setEnabled(true);
    // ui->actionFileCloseDb->setEnabled(true);
    setFileMenuOptions(0,1,0,1,1);
}

void MainWindow::on_actionFileOpenDb_triggered()
{
    ui->view->setEnabled(false);

    QMessageBox msgBox;
    msgBox.setText("Do you want to Create a New Database or Append to Existing?");
    msgBox.setInformativeText("Please choose one of the following actions.");

    // Define the standard buttons for the two options
    QPushButton *optionNewButton = msgBox.addButton("Create New DB", QMessageBox::ActionRole);
    QPushButton *optionLoadButton = msgBox.addButton("Load Existing DB", QMessageBox::ActionRole);
    QPushButton *optionAppendButton = msgBox.addButton("Append to DB", QMessageBox::ActionRole);

    // Optionally set a default button
    msgBox.setDefaultButton(optionAppendButton);

    // Execute the message box and get the clicked button
    msgBox.exec();

    // Check which button was clicked
    if (msgBox.clickedButton() == optionNewButton) {
        qDebug().noquote() << "User chose to Create a new Db";
        // Perform actions for Option A
        on_actionFileCloseDb_triggered();
    } else if (msgBox.clickedButton() == optionLoadButton) {
        qDebug().noquote() << "User chose to Load Existing Db";
        loadExistingDatabase();
        return;
    } else if (msgBox.clickedButton() == optionAppendButton) {
        qDebug().noquote() << "User chose to Append";
    } else {
        qDebug().noquote() << "Message box closed without a choice.";
        return;
    }

    // QMessageBox::critical(this, "Not Implemented", "This function is not implemented!", QMessageBox::Ok);

    qDebug().noquote() << " Entering setupDatabase()";
    setupDatabase();

    if (!db.open()) { qDebug().noquote() << "DB NOT OPENED"; }

    qDebug().noquote() << " Entering createTable()";
    createTable();

    // ui->actionFileReadDb->setEnabled(true);
    // ui->actionFileWriteDb->setEnabled(false);
    // ui->actionFileOpenDb->setEnabled(false);
    // ui->actionFileDisplayDb->setEnabled(false);
    // ui->actionFileCloseDb->setEnabled(true);
    setFileMenuOptions(1,0,0,0,1);
}

void MainWindow::on_actionFileExit_triggered() {
    QApplication::exit();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox msgBox;
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setInformativeText("AdifToSqlite");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setWindowTitle("HTML Message");
    msgBox.setText("<b>ADIF to SQLITE v0.3</b><br/><br/>"
                "Demonstration software and "
                "technique refinement exercise.");
    msgBox.setInformativeText("Version 0.3 - &copy; 2025 Steve VK3VM");
    msgBox.exec();
}
