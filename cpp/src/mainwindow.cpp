#include <QJsonArray>

#include "mainwindow.hpp"
#include "./ui_mainwindow.h"
#include "Scene.hpp"
#include "Token.hpp"
#include "game.hpp"

MainWindow::MainWindow(Game *game, QWidget *parent)
    : QMainWindow(parent)
    , scene(new Scene(game, this))
    , game(game)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(onActionExit()));
    connect(ui->actionRestart, SIGNAL(triggered(bool)), this, SLOT(onActionRestart()));
    connect(ui->actionLoad, SIGNAL(triggered(bool)), this, SLOT(onActionLoad()));
    connect(ui->actionSave, SIGNAL(triggered(bool)), this, SLOT(onActionSave()));
    connect(ui->actionDevMode, SIGNAL(toggled(bool)), this, SLOT(onActionDevMode()));
    connect(ui->actionDevMode, SIGNAL(toggled(bool)), this, SLOT(reset()));
    connect(ui->actionShowMask, SIGNAL(toggled(bool)), this, SLOT(onActionShowMask()));

    connect(ui->actionShowFreeTow, SIGNAL(triggered(bool)), this, SLOT(onActionShowFreeTow()));
    connect(ui->actionShowFreeThree, SIGNAL(triggered(bool)), this, SLOT(onActionShowFreeThree()));
    connect(ui->actionShowFreeFour, SIGNAL(triggered(bool)), this, SLOT(onActionShowFreeFour()));

    connect(ui->actionShowWin, SIGNAL(triggered(bool)), this, SLOT(onActionShowWin()));
    connect(ui->actionShowCapture, SIGNAL(triggered(bool)), this, SLOT(onActionShowCapture()));
    connect(ui->actionShowUnderCapture, SIGNAL(triggered(bool)), this, SLOT(onActionShowUnderCapture()));
    connect(ui->actionShowTowFreeThree, SIGNAL(triggered(bool)), this, SLOT(onActionShowTowFreeThree()));
    connect(ui->actionHelpWithMove, SIGNAL(triggered(bool)), this, SLOT(onActionHelpWithMove()));


    connect(scene, SIGNAL(resetted()), this, SLOT(reset()));

    ui->graphicsView->setScene(scene);
    ui->graphicsView->setMouseTracking(true);
    scene->addText("Hello World");
    scene->setSceneRect(ui->graphicsView->rect());
    setWindowTitle("Gomoku");
    SetAiTitle();
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onActionExit() {
    qDebug() << "onActionExit" << this;
    exit(0);
}

bool MainWindow::isDevMode() {
    return ui->actionDevMode->isChecked();
}

void MainWindow::onActionRestart() {
    qDebug() << "onActionRestart" << this;
    if (!game)
        return;
    game->reset();
    scene->reset();
    scene->startGame();
}

void MainWindow::onActionLoad() {
    auto f = QFileDialog::getOpenFileName(
        this,
        "Select board saved to .json file!"
    );
    qDebug() << "onActionLoad" << f;
    if (f.isNull())
        return;
    QFile file;
    QString data;
    file.setFileName(f);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    data = file.readAll();
    file.close();
    auto doc = QJsonDocument::fromJson(data.toUtf8());
    if (doc.isNull() || !doc.isArray())
        return;
    auto obj = doc.array();
    if (obj.size() != 19)
        return;
    for (const auto &item : obj) {
        if (!item.isArray())
            return;
        auto row = item.toArray();
        if (row.size() != 19)
            return;
    }
    auto y = 0;
    for (const auto &item : obj)
    {
        auto x = 0;
        auto row = item.toArray();
        for (const auto &val : row)
        {

            auto v = val.toInt(0);
            Scene::TokenDef td(Scene::tokenColorFromInt(v),
                                 QColor(Qt::transparent),
                                 QString());
            scene->setToken(x, y, td);
            x++;
        }
        y++;
    }
    scene->game->board.print();
}

void MainWindow::onActionSave() {
    qDebug() << "onActionSave";
    QJsonArray rows;
    for (int i = 0; i < GSIZE; ++i) {
        QJsonArray row;
        for (int j = 0; j < GSIZE; ++j) {
            auto t = scene->getToken(j, i);
            row.append(Scene::intFromTokenColor(t->def.color));
        }
        rows.append(row);
    }
    QJsonDocument doc;
    doc.setArray(rows);
    QFile file(QFileDialog::getSaveFileName(
            this,
            "File to save board to!"
    ));
    file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
    if (!file.isWritable())
        return;
    file.write(doc.toJson());
    file.close();
}

void MainWindow::onActionDevMode() {
    scene->devMode = ui->actionDevMode->isChecked();
    qDebug() << "onActionDevMode" << ui->actionDevMode->isChecked();
    if (scene->devMode)
        setWindowTitle("Gomoku (DEV MODE)");
    else
        setWindowTitle("Gomoku");
}

void MainWindow::SetAiTitle() {
    ui->aiTitle->setText(QString(
                "<html><head/><body>"
                "<h1>Hi there!</h1>"
                "<p>Gomoku will crush you!</p>"
                "<p>Last Move took: %1 sec</p>"
                "<p>Cache count: %2 </p>"
                "<p>Cache hit count: %3 </p>"
                "<p>Prune count: %4 </p>"
                "<p>Node count: %5 </p>"
                "<p>Black captures: %6 </p>"
                "<p>White captures: %7 </p>"
                "<p>Game in dev mode: %8 </p>"
                "</body></html>"
        )
        .arg(
                QString::number(scene->lastPredictedMove.tookSecond, 'g', 4),
                QString::number(scene->game->board.hash_map.size()),
                QString::number(scene->game->board.cache_hit_count),
                QString::number(scene->game->board.pruned_count),
                QString::number(scene->game->board.nodes_count),
                QString::number(scene->game->board.black_captures_count),
                QString::number(scene->game->board.white_captures_count),
                scene->devMode ? "<span style=\" color:#cc0000;\">True</span>" : "False"
        )
    );
}

void MainWindow::onActionShowMask() {
    scene->showMask = ui->actionShowMask->isChecked();
    qDebug() << "onActionShowMask" << scene->showMask;
    scene->reset();
}

void MainWindow::onActionPvPMode() {
    scene->pvpMode = ui->actionPvPMode->isChecked();
    qDebug() << "onActionPvPMode" << scene->pvpMode;
    scene->reset();
}

void MainWindow::onActionShowFreeFour() {
    for (int y = 0; y < GSIZE; ++y) {
        for (int x = 0; x < GSIZE; ++x) {
            if (game->getToken(x, y))
                continue;
            auto match = false;
            for (const auto &dxy : ALL_DIRS) {
                char flat[BOARD_SIZE];
                Patterns::getFlat(
                        x, y,
                        dxy[0], dxy[1],
                        Scene::WHITE, 5,
                        game->board.black_board,
                        game->board.white_board,
                        flat
                );
                if (Patterns::isFourFree(Scene::WHITE, flat)) {
                    match = true;
                    break;
                }
            }
            auto t = scene->getToken(x, y);
            t->setDef({
                              t->def.color,
                              QColor(!match ? Qt::transparent : Qt::yellow),
                              t->def.text}
            );
            t->update();
        }
    }
    scene->update();
}

void MainWindow::onActionShowFreeTow() {
    for (int y = 0; y < GSIZE; ++y) {
        for (int x = 0; x < GSIZE; ++x) {
            if (game->getToken(x, y))
                continue;
            auto match = false;
            for (const auto &dxy : ALL_DIRS) {
                char flat[BOARD_SIZE];
                Patterns::getFlat(
                        x, y,
                        dxy[0], dxy[1],
                        Scene::WHITE, 3,
                        game->board.black_board,
                        game->board.white_board,
                        flat
                );
                if (Patterns::isTowFree(Scene::WHITE, flat)) {
                    match = true;
                    break;
                }
            }
            auto t = scene->getToken(x, y);
            t->setDef({
                              t->def.color,
                              QColor(!match ? Qt::transparent : Qt::yellow),
                              t->def.text}
            );
            t->update();
        }
    }
    scene->update();
}

void MainWindow::onActionShowFreeThree() {
    for (int y = 0; y < GSIZE; ++y) {
        for (int x = 0; x < GSIZE; ++x) {
            auto t = scene->getToken(x, y);
            if (game->getToken(x, y))
                continue;
            auto match = false;
            for (const auto &dxy : ALL_DIRS) {
                char flat[BOARD_SIZE];
                Patterns::getFlat(
                        x, y,
                        dxy[0], dxy[1],
                        Scene::WHITE, 5,
                        game->board.black_board,
                        game->board.white_board,
                        flat
                );
                if (Patterns::isThreeFree(Scene::WHITE, flat)) {
                    match = true;
                    break;
                }
            }

            t->setDef({
                              t->def.color,
                              QColor(!match ? Qt::transparent : Qt::yellow),
                              t->def.text}
            );
            t->update();
        }
    }
    scene->update();
}

void MainWindow::reset() {
    SetAiTitle();
}

void MainWindow::onActionShowWin() {
    for (int y = 0; y < GSIZE; ++y) {
        for (int x = 0; x < GSIZE; ++x) {
            auto t = scene->getToken(x, y);
            if (!game->getToken(x, y))
                continue;
            auto match = game->board.PtrLocal5Match(Board::BLACK, x, y);
            t->setDef({
                              t->def.color,
                              QColor(!match ? Qt::transparent : Qt::yellow),
                              t->def.text}
            );
            t->update();
        }
    }
    scene->update();
}

void MainWindow::onActionShowCapture() {
    for (int y = 0; y < GSIZE; ++y) {
        for (int x = 0; x < GSIZE; ++x) {
            auto t = scene->getToken(x, y);
            if (game->getToken(x, y))
                continue;
            auto match = false;
            for (const auto &dxy : ALL_DIRS) {
                char flat[BOARD_SIZE];
                Patterns::getFlat(
                        x, y,
                        dxy[0], dxy[1],
                        Scene::WHITE, 3,
                        game->board.black_board,
                        game->board.white_board,
                        flat
                );
                if (Patterns::isCapture(Scene::WHITE, flat)) {
                    match = true;
                    break;
                }
            }

            t->setDef({
                              t->def.color,
                              QColor(!match ? Qt::transparent : Qt::yellow),
                              t->def.text}
            );
            t->update();
        }
    }
    scene->update();
}

void MainWindow::onActionShowUnderCapture() {
    for (int y = 0; y < GSIZE; ++y) {
        for (int x = 0; x < GSIZE; ++x) {
            auto t = scene->getToken(x, y);
            if (game->getToken(x, y))
                continue;
            auto match = false;
            for (const auto &dxy : ALL_DIRS) {
                char flat[BOARD_SIZE];
                Patterns::getFlat(
                        x, y,
                        dxy[0], dxy[1],
                        Scene::WHITE, 3,
                        game->board.black_board,
                        game->board.white_board,
                        flat
                );
                if (Patterns::isUnderCapture(Scene::WHITE, flat)) {
                    match = true;
                    break;
                }
            }

            t->setDef({
                              t->def.color,
                              QColor(!match ? Qt::transparent : Qt::yellow),
                              t->def.text}
            );
            t->update();
        }
    }
    scene->update();
}

void MainWindow::onActionShowTowFreeThree() {
    for (int y = 0; y < GSIZE; ++y) {
        for (int x = 0; x < GSIZE; ++x) {
            auto t = scene->getToken(x, y);
            if (game->getToken(x, y))
                continue;
            auto match = 0;
            for (const auto &dxy : ALL_DIRS) {
                char flat[BOARD_SIZE];
                Patterns::getFlat(
                        x, y,
                        dxy[0], dxy[1],
                        Scene::WHITE, 5,
                        game->board.black_board,
                        game->board.white_board,
                        flat
                );
                if (Patterns::isThreeFree(Scene::WHITE, flat)) {
                    match++;
                }
            }
            t->setDef({
                  t->def.color,
                  QColor(match < 2 ? Qt::transparent : Qt::yellow),
                  t->def.text}
            );
            t->update();
        }
    }
    scene->update();
}

void MainWindow::quit() {
    qDebug() << "quit" << this;
    exit(0);
}

void MainWindow::onActionHelpWithMove() {
    scene->onHelpMove();
}
