#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QMessageBox>
#include <stdexcept>
#include <assert.h>

MainWindow::MainWindow(QWidget *parent):
	QMainWindow(parent), _ui(new Ui::DG_Timer), _timer(new QTimer(this))
{
	_ui->setupUi(this);
	_onEnter(_state = State::IDLE);
	connect(_timer, SIGNAL(timeout()), this, SLOT(onUpdate()));
	connect(_ui->fiveButton, SIGNAL(clicked()), this, SLOT(set5min()));
	connect(_ui->twofiftyButton, SIGNAL(clicked()), this, SLOT(set25min()));
	connect(_ui->fourfiftyButton, SIGNAL(clicked()), this, SLOT(set45min()));
	connect(_ui->timerButton, SIGNAL(clicked()), this, SLOT(pushTButton()));
}

void MainWindow::_swState(State st) {
	_onExit(st);
	std::swap(_state, st);
	_onEnter(st);
}
void MainWindow::_onEnter(State st) {
	switch(_state) {
		case State::IDLE:
			// カウント終了後から来た場合はカウント値をリセット
			if(st == State::COUNTEND)
				_ui->timeEdit->setTime(QTime(0,0,0));
			// ボタンのセットアップ
			_ui->progressBar->setValue(0);
			_ui->timerButton->setText("start");
			_ui->timeEdit->setEnabled(true);
			_ui->timeGroup->setEnabled(true);
			setWindowTitle("DG_Timer: idle");
			break;
		case State::COUNTING:
			_ui->timerButton->setText("stop");
			_timer->start(1000);
			break;
		case State::COUNTEND:
			// タイマーゼロ表示
			_ui->timeEdit->setTime(QTime(0,0,0));
			_ui->progressBar->setValue(100);
			_ui->timerButton->setText("reset");
			// ウィンドウのフラッシュ処理 (手抜きバージョン。本当はOS固有の処理をする)
			this->hide();
			this->show();
			setWindowTitle("DG_Timer: ended");
			break;
	}
}
void MainWindow::_onExit(State st) {
	if(_state == State::IDLE) {
		_ui->timeEdit->setEnabled(false);
		_ui->timeGroup->setEnabled(false);
	} else if(_state == State::COUNTING)
		_timer->stop();
}
MainWindow::~MainWindow() {
	delete _ui;
}

void MainWindow::set5min() {
	assert(_state == State::IDLE);
	_ui->timeEdit->setTime(QTime(0,5,0));
}
void MainWindow::set25min() {
	assert(_state == State::IDLE);
	_ui->timeEdit->setTime(QTime(0,25,0));
}
void MainWindow::set45min() {
	assert(_state == State::IDLE);
	_ui->timeEdit->setTime(QTime(0,45,0));
}

void MainWindow::_setDuration(QTime tm) {
	int sec = tm.second(),
		minute = tm.minute(),
		hour = tm.hour();
	if(tm.isValid() && (sec>0 || minute>0 || hour>0))
		_setDuration(std::chrono::duration_cast<TDur>(
				std::chrono::seconds(sec+minute*60+hour*3600)));
	else
		throw std::runtime_error("invalid time");
}
void MainWindow::_setDuration(TDur dur) {
	assert(_state == State::IDLE);
	_beginTime = Clock::now();
	_dstTime = _beginTime + dur;
	_durSec = std::chrono::duration_cast<std::chrono::seconds>(dur).count();
}

void MainWindow::onUpdate() {
	using std::chrono::seconds;
	using std::chrono::minutes;
	using std::chrono::hours;
	using std::chrono::milliseconds;
	using std::chrono::duration_cast;

	// 残り時間を表示
	auto remain = _dstTime - Clock::now();
	if(remain <= milliseconds(0))
		_swState(State::COUNTEND);
	else {
		// 残り時間を表示
		int ratio = (_durSec-duration_cast<seconds>(remain).count())*100 / _durSec;
		_ui->progressBar->setValue(ratio);

		int hour = duration_cast<hours>(remain).count(),
			minute = duration_cast<minutes>(remain).count() % 60,
			sec = duration_cast<seconds>(remain).count() % 60;
		QTime tm(hour, minute, sec);
		_ui->timeEdit->setTime(tm);

		// タイトルバーに時間を表示
		this->setWindowTitle(QString("DG_Timer: %1 - %2%").arg(tm.toString("HH:mm:ss")).arg(ratio));
	}
}

void MainWindow::pushTButton() {
	switch(_state) {
		case State::IDLE: {
				try {
					_setDuration(_ui->timeEdit->time());
					_swState(State::COUNTING);
				} catch(const std::exception& e) {
					QMessageBox::warning(this, "error", e.what(), QMessageBox::Ok);
				}
			break; }
		case State::COUNTING:
			// カウントダウンを取り消してIDLE状態に戻る
			_swState(State::IDLE);
			break;
		case State::COUNTEND:
			// IDLE状態に戻る
			_swState(State::IDLE);
			break;
	}
}
