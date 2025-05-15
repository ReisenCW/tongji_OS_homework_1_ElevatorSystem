#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ElevatorSystem.h"
#include <qpushbutton.h>


class ElevatorSystem : public QMainWindow
{
    Q_OBJECT

public:
    ElevatorSystem(QWidget *parent = nullptr);
	~ElevatorSystem() {}
public:
	int GetFloorCount() const { return floor_count; }
	int GetElevatorCount() const { return elevator_count; }
private:
	void BeginSimulation(); // 开始模拟
	void InitWidget(); // 初始化
	void ResetParams() {
		elevator_count = 5;
		floor_count = 20;
	}
public slots:
	void HandleSimulationClosed() {
		setVisible(true);
	}

private:
    Ui::ElevatorSystemClass ui;
	int elevator_count; // 电梯数量
	int floor_count; // 楼层数量
};
