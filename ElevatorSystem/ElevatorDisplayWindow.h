#pragma once

#include <QScrollArea>
#include <QWidget>
#include <qlabel.h>
#include <SimulationMainWindow.h>
#include "ui_ElevatorDisplayWindow.h"
#include <vector>
class Elevator;

class ElevatorDisplayWindow : public QWidget
{
	Q_OBJECT

public:
	ElevatorDisplayWindow(int elevatorCount, int floorCount, QLabel* elevator_floor_labels, SimulationMainWindow* simu_window, QWidget* parent = nullptr);
	~ElevatorDisplayWindow() {}

private:
	Ui::ElevatorDisplayWindowClass ui;

public slots:
	void HandleSimulationClosed() {
		this->close();
	}
public:
	SimulationMainWindow* simu_window;
private:
	QScrollArea* scrollArea;
	QLabel* elevator_floor_labels;
	int elevator_count;
	int floor_count;
};
