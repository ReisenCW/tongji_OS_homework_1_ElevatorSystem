#include "ElevatorDisplayWindow.h"
#include "Elevator.h"

ElevatorDisplayWindow::ElevatorDisplayWindow(int elevatorCount, int floorCount, QLabel* elevator_floor_labels, SimulationMainWindow* simu_window, QWidget* parent)
    : QWidget(parent, Qt::Window), elevator_count(elevatorCount), elevator_floor_labels(elevator_floor_labels), simu_window(simu_window), floor_count(floorCount) {
    setWindowTitle("电梯监控界面");
    setMinimumSize(1100, 700);
	move(100, 100); // 设置窗口初始位置
    scrollArea = new QScrollArea(this);
    QWidget* container = new QWidget(scrollArea);
    QGridLayout* layout = new QGridLayout(container);

    // 每行显示3个电梯
    for (int i = 0; i < elevatorCount; ++i) {
        Elevator* elevator = new Elevator(i + 1, floorCount, &elevator_floor_labels[i], container);
        elevator->Init();
        layout->addWidget(elevator, i / 3, i % 3);
        elevator->show();
		simu_window->AddElevator(elevator);
    }

    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(scrollArea);
}