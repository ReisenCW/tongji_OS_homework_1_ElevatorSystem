#include "ElevatorSystem.h"
#include <SimulationMainWindow.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qlineedit.h>
ElevatorSystem::ElevatorSystem(QWidget *parent)
    : QMainWindow(parent), elevator_count(5), floor_count(20)
{
    ui.setupUi(this);
    this->setFixedSize(480, 360);
	InitWidget();
}


void ElevatorSystem::InitWidget() {
	//创建控件
	QLabel* e_label = new QLabel("电梯数量 elevator numbers: ", this);
	e_label->setGeometry(80, 50, 200, 50);
	QLabel* f_label = new QLabel("楼层数量 floor numbers: ", this);
	f_label->setGeometry(80, 150, 200, 50);

	QLineEdit* e_edit = new QLineEdit(this);
	e_edit->setText(QString::number(this->elevator_count));
	e_edit->setPlaceholderText("请输入电梯数量");
	e_edit->setAlignment(Qt::AlignCenter);
	e_edit->setGeometry(300, 50, 100, 50);

	QLineEdit* f_edit = new QLineEdit(this);
	f_edit->setText(QString::number(this->floor_count));
	f_edit->setPlaceholderText("请输入楼层数量");
	f_edit->setAlignment(Qt::AlignCenter);
	f_edit->setGeometry(300, 150, 100, 50);

	//创建按钮
	QPushButton* start_button = new QPushButton("开始模拟", this);
	start_button->setGeometry(120, 280, 100, 40);

	QPushButton* reset_button = new QPushButton("重置参数", this);
	reset_button->setGeometry(260, 280, 100, 40);

	//连接信号和槽
	connect(start_button, &QPushButton::clicked, this, [=]() {
			bool ok1, ok2;
			int elevator_count = e_edit->text().toInt(&ok1);
			int floor_count = f_edit->text().toInt(&ok2);
			if (ok1 && ok2) {
				this->elevator_count = elevator_count;
				this->floor_count = floor_count;
				BeginSimulation();
			}
			else {
				QMessageBox::warning(this, "错误", "请输入有效的数字");
				ResetParams();
				e_edit->setText(QString::number(this->elevator_count));
				f_edit->setText(QString::number(this->floor_count));
			}
		}
	);

	connect(reset_button, &QPushButton::clicked, this, [=]() {
			ResetParams();
			e_edit->setText(QString::number(this->elevator_count));
			f_edit->setText(QString::number(this->floor_count));
		}
	);
}

void ElevatorSystem::BeginSimulation() {
	setVisible(false);
	SimulationMainWindow* simulation_window = new SimulationMainWindow(this);
	simulation_window->show();
}