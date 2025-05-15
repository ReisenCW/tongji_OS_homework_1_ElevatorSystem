#include "Elevator.h"

Elevator::Elevator(int elevator_id, int floor_cnt, QLabel* elevator_floor, QWidget *parent)
	: QWidget(parent), elevator_id(elevator_id), floor_cnt(floor_cnt), 
    current_floor(0), state(ElevatorState::Idle), elevator_floor(elevator_floor),
    is_door_open(false)
{
	ui.setupUi(this);
    Init();
}


void Elevator::Init()
{
	this->setFixedSize(280, 480);
	this->InitParams();
	this->InitWidget();
    this->UpdateDisplay();
}

void Elevator::InsertTargetFloor(int floor) {
    if (floor == current_floor) return;
	// 如果queue中已经有该楼层请求，则不重复添加
	if (RequestExists(floor)) {
		return;
	}
    // 若队列为空或方向未定，直接插入
    if (target_floors.empty()) {
        target_floors.push(floor);
        return;
    }

    // 提取队列内容到临时向量
    std::vector<int> temp;
    while (!target_floors.empty()) {
        temp.push_back(target_floors.front());
        target_floors.pop();
    }

    // 根据当前方向插入排序
    if (state == ElevatorState::Up ||
        (state == ElevatorState::Idle && floor > current_floor)) {
        // 上行方向：插入到第一个小于等于floor的位置之后
        auto pos = std::upper_bound(temp.begin(), temp.end(), current_floor);
        temp.insert(pos, floor);
    }
    else {
        // 下行方向：插入到第一个大于等于floor的位置之前
        auto pos = std::lower_bound(temp.begin(), temp.end(), current_floor, std::greater<int>());
        temp.insert(pos, floor);
    }

    // 重新填充队列
    for (int f : temp) {
        target_floors.push(f);
    }
}

void Elevator::InitParams()
{
	current_floor = 0; // 初始楼层
	state = ElevatorState::Idle; // 初始状态
}

void Elevator::InitWidget() {
    // 主容器
    QWidget* mainContainer = new QWidget();
    mainContainer->setObjectName("mainContainer");
    mainContainer->setStyleSheet(
        "QWidget#mainContainer {"
        "   border: 1px solid red;"
        "   border-radius: 10px;"
        "}"
    );
    QVBoxLayout* mainLayout = new QVBoxLayout(mainContainer);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setAlignment(Qt::AlignTop);

    // 标题
    QLabel* titleLabel = new QLabel(QString("电梯 %1").arg(elevator_id), mainContainer);
    titleLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: #333;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 主体布局（滑动条 + 右侧区域）
    QHBoxLayout* bodyLayout = new QHBoxLayout();
    mainLayout->addLayout(bodyLayout, 1);

    // --- 左侧滑动条区域 ---
    QSlider* floorSlider = new QSlider(Qt::Vertical, mainContainer);
    floorSlider->setRange(1, floor_cnt);
    floorSlider->setValue(current_floor + 1);
    floorSlider->setEnabled(false);
    floorSlider->setStyleSheet(
        "QSlider::groove:vertical {"
        "   border: 1px solid #999;"
        "   width: 10px;"
        "   margin-top: 10px;"
        "}"
        "QSlider::handle:vertical {"
        "   background: #555;"
        "   height: 3px;"
        "   margin: 0 -4px;"
        "}"
    );

    QLabel* floorValueLabel = new QLabel(QString::number(current_floor + 1), mainContainer);
    floorValueLabel->setObjectName("floorValueLabel");
    floorValueLabel->setAlignment(Qt::AlignCenter);
    floorValueLabel->setStyleSheet("font-weight: bold; color: blue;");

    QVBoxLayout* sliderLayout = new QVBoxLayout();
    sliderLayout->addWidget(floorSlider, 1);
    sliderLayout->addWidget(floorValueLabel);
    bodyLayout->addLayout(sliderLayout);

    // --- 右侧区域（按钮 + 状态）---
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(10);
    bodyLayout->addLayout(rightLayout, 1); // 右侧区域占剩余空间

    // 楼层按钮网格
    QGridLayout* btnGrid = new QGridLayout();
    btnGrid->setSpacing(3);
    const int BUTTONS_PER_ROW = 5;
    for (int i = 0; i < floor_cnt; ++i) {
        QPushButton* btn = new QPushButton(QString::number(i + 1), mainContainer);
        btn->setFixedSize(40, 40);
        btn->setStyleSheet(
            "QPushButton {"
            "   border: 1px solid #666;"
            "   border-radius: 5px;"
            "   background-color: white;" // 默认白色
            "}"
            "QPushButton:disabled {"      // 禁用状态变红
            "   background-color: red;"
            "}"
            "QPushButton:pressed {"
			"   background-color: #ccc;"
			"   border: 1px solid #999;"
			"   color: #fff;"
			"}"
        );
        connect(btn, &QPushButton::clicked, this, [=]() {
            int target_floor = i + 1; // 假设按钮标签为1-based
            this->AddTargetFloor(target_floor - 1); // 若电梯空闲则启动
        });
        floorButtons.push_back(btn);
        btnGrid->addWidget(btn, i / BUTTONS_PER_ROW, i % BUTTONS_PER_ROW);
    }
    rightLayout->addLayout(btnGrid);

    // 门按钮
    QHBoxLayout* doorLayout = new QHBoxLayout();
    QPushButton* openBtn = CreateDoorButton("开门", "lightgreen");
    QPushButton* closeBtn = CreateDoorButton("关门", "lightcoral");
    connect(openBtn, &QPushButton::clicked, this, &Elevator::HandleOpenDoor);
    connect(closeBtn, &QPushButton::clicked, this, &Elevator::HandleCloseDoor);
    doorLayout->addWidget(openBtn);
    doorLayout->addWidget(closeBtn);
    rightLayout->addLayout(doorLayout);

    // 状态栏
    QGroupBox* statusGroup = CreateStatusGroup();
    rightLayout->addWidget(statusGroup);

    // 设置主窗口布局
    QVBoxLayout* windowLayout = new QVBoxLayout(this);
    windowLayout->setContentsMargins(0, 0, 0, 0);      // 清除默认边距
    windowLayout->addWidget(mainContainer);
    this->setLayout(windowLayout);
}

void Elevator::AddTargetFloor(int floor)
{
    if (floor == current_floor) return;
    InsertTargetFloor(floor);
    int button_index = floor;
    if (button_index >= 0 && button_index < floorButtons.size()) {
        QPushButton* btn = floorButtons[button_index];
        btn->setDisabled(true); // 禁用按钮
    }
    if (state == ElevatorState::Idle) {
        // 启动电梯移动
        state = (floor > current_floor) ? ElevatorState::Up : ElevatorState::Down;
        QTimer::singleShot(600, this, [=]() { MoveToNextFloor(); });
    }
}

// 辅助函数：创建门按钮
QPushButton* Elevator::CreateDoorButton(const QString& text, const QString& color) {
    QPushButton* btn = new QPushButton(text, this);
    btn->setFixedSize(50, 20);
    btn->setStyleSheet(
        QString("QPushButton {"
            "   background-color: %1;"
            "   border: 1px solid #666;"
            "   border-radius: 5px;"
            "}"
            "QPushButton:pressed { background-color: %2; }"
        ).arg(color).arg(color == "lightgreen" ? "#7cb342" : "#e57373")
    );
    return btn;
}

// 辅助函数：创建状态栏
QGroupBox* Elevator::CreateStatusGroup() {
    QGroupBox* group = new QGroupBox("电梯状态", this);
    // 标题居中,字体较大
	group->setStyleSheet("QGroupBox::title { text-align: center; font-size: 24px;}");
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setSpacing(5);

    const QList<QPair<QString, QString>> statusLabels = {
        {"状态:", "           空闲"}, {"当前楼层:", "   0"}, {"门状态:", "       关闭"}
    };

    for (const auto& pair : statusLabels) {
        QLabel* label = new QLabel(pair.first  + pair.second, group);
        label->setStyleSheet("font-size: 20px; color: #444; text-decoration: underline;");
        layout->addWidget(label);
        if (pair.first == "状态:") label->setObjectName("stateLabel");
        else if (pair.first == "当前楼层:") label->setObjectName("floorLabel");
        else if (pair.first == "门状态:") label->setObjectName("doorLabel");
    }

    group->setStyleSheet(
        "QGroupBox {"
        "   border: 1px solid #999;"
        "   border-radius: 5px;"
        "   padding: 16px;"
        "}"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px;}"
    );
    return group;
}

bool Elevator::RequestExists(int floor)
{
	// 检查目标队列中是否存在该请求
	std::queue<int> temp = target_floors;
	while (!temp.empty()) {
		if (temp.front() == floor) {
			return true;
		}
		temp.pop();
	}
    return false;
}

void Elevator::HandleOpenDoor() {
    //ClearAllTimers(); // 清除所有定时器

    if (state == ElevatorState::Idle ||
        state == ElevatorState::Closing ||
        state == ElevatorState::Open)
    {
        state = ElevatorState::Opening;
        UpdateDisplay();
        int arrived_floor = current_floor; // 直接使用current_floor作为索引
        if (arrived_floor >= 0 && arrived_floor < floorButtons.size()) {
            QPushButton* btn = floorButtons[arrived_floor];
            btn->setDisabled(false);
        }
        // 开门过程定时器
        m_openTimer = new QTimer(this);
        m_openTimer->singleShot(1000, [=]() {
            state = ElevatorState::Open;
            UpdateDisplay();

            // 自动关门定时器
            m_stayOpenTimer = new QTimer(this);
            m_stayOpenTimer->singleShot(2000, [=]() {
                state = ElevatorState::Closing;
                UpdateDisplay();

                // 关门过程定时器
                m_closeTimer = new QTimer(this);
                m_closeTimer->singleShot(1000, [=]() {
                    state = ElevatorState::Idle;
                    UpdateDisplay();
                });
             });
        });
    }
}

void Elevator::HandleCloseDoor() {
    //ClearAllTimers(); // 清除所有定时器

    if (state == ElevatorState::Open ||
        state == ElevatorState::Opening)
    {
        state = ElevatorState::Closing;
        UpdateDisplay();

        // 直接进入关门流程
        m_closeTimer = new QTimer(this);
        m_closeTimer->singleShot(1000, [=]() {
            state = ElevatorState::Idle;
            UpdateDisplay();
        });
    }
}

void Elevator::MoveToNextFloor()
{
	// 如果电梯正在开门或关门，则不进行移动
    if (state == ElevatorState::Opening || state == ElevatorState::Open 
        || state == ElevatorState::Closing)
        return;
    //若目标队列为空，回到IDLE
    if (target_floors.empty()) {
        state = ElevatorState::Idle;
        return;
    }
	// 获取下一个目标楼层
    int next_floor = target_floors.front();
    if (current_floor < next_floor) current_floor++;
    else if (current_floor > next_floor) current_floor--;

    // 更新上下行状态
    if (current_floor < next_floor) {
        state = ElevatorState::Up;
    }
    else if (current_floor > next_floor) {
        state = ElevatorState::Down;
    }
    // 更新开关门状态
    if (current_floor == next_floor) {
        // 触发到达信号
        ElevatorState next_direction = ElevatorState::Idle;
        QString debug_str = "idle";
		// 如果目标队列不为空, 根据下一个目标楼层决定方向
		if (!target_floors.empty()) {
			int next_target = target_floors.front();
			if (next_target > current_floor) {
				next_direction = ElevatorState::Up;
				debug_str = "up";
			}
			else if (next_target < current_floor) {
				next_direction = ElevatorState::Down;
				debug_str = "down";
			}
		}
		qDebug() << "next_direction:" << debug_str;
        emit FloorArrived(current_floor, next_direction); 
		// 到达目标楼层，清除目标队列
        target_floors.pop();
		qDebug() << "电梯" << elevator_id << "到达" << current_floor + 1 << "楼";
        //ClearAllTimers(); // 清除残留定时器
        int button_index = current_floor;
        if (button_index >= 0 && button_index < floorButtons.size()) {
            QPushButton* btn = floorButtons[button_index];
            btn->setDisabled(false); // 启用按钮
        }

        state = ElevatorState::Opening;
        UpdateDisplay();

        m_openTimer = new QTimer(this);
        m_openTimer->singleShot(1000, [=]() {
            state = ElevatorState::Open;
            UpdateDisplay();

            m_stayOpenTimer = new QTimer(this);
            m_stayOpenTimer->singleShot(2000, [=]() {
                state = ElevatorState::Closing;
                UpdateDisplay();

                m_closeTimer = new QTimer(this);
                m_closeTimer->singleShot(1000, [=]() {
                    state = ElevatorState::Idle;
                    UpdateDisplay();
                    //QMessageBox::information(this, "到达", QString("电梯 %1 到达 %2 楼").arg(elevator_id).arg(current_floor + 1));

                    if (!target_floors.empty()) {
                        QTimer::singleShot(600, this, &Elevator::MoveToNextFloor);
                    }
                });
            });
        });
    }
    else {
        QTimer::singleShot(600, this, &Elevator::MoveToNextFloor);
    }

    // 更新界面
    elevator_floor->setText(QString::number(current_floor + 1));
    elevator_floor->repaint(); // 强制重绘
    UpdateDisplay();
}

void Elevator::UpdateDisplay()
{
    // 更新滑动条
    QSlider* floorSlider = findChild<QSlider*>();
    if (floorSlider) {
        floorSlider->setValue(current_floor + 1);
    }
	QLabel* floorValueLabel = findChild<QLabel*>("floorValueLabel");
	if (floorValueLabel) {
		floorValueLabel->setText(QString::number(current_floor + 1));
	}
	// 更新电梯按钮
	for (int i = 0; i < floor_cnt; ++i) {
		QPushButton* btn = findChild<QPushButton*>(QString("btn_%1").arg(i + 1));
		if (btn) {
			btn->setEnabled(current_floor != i);
		}
	}
	// 更新当前状态

    // 状态颜色控制
    QString state_color = "#444";

    // 更新上下行状态
	state_color = state == ElevatorState::Idle ? "#444" : "red";
    QLabel* stateLabel = findChild<QLabel*>("stateLabel");
    if (stateLabel) {
        QString state_str;
        switch (state) {
        case ElevatorState::Idle: state_str = "空闲"; break;
        case ElevatorState::Up: state_str = "上行"; break;
        case ElevatorState::Down: state_str = "下行"; break;
        case ElevatorState::Opening: state_str = "开门中"; break;
        case ElevatorState::Open: state_str = "已开门"; break;
        case ElevatorState::Closing: state_str = "关门中"; break;
        }
        stateLabel->setText(QString("状态:          %1").arg(state_str));
        stateLabel->setStyleSheet(QString("color: %1; font-size: 20px;").arg(state_color));
    }
	// 更新电梯所在楼层
    switch (state) {
    case ElevatorState::Up:
    case ElevatorState::Down:
        state_color = "red";
        break;
    default:
        state_color = "#444";
        break;
    }
    QLabel* floorLabel = findChild<QLabel*>("floorLabel");
    if (floorLabel) {
        floorLabel->setText(QString("当前楼层:   %1 楼").arg(current_floor + 1));
		floorLabel->setStyleSheet(QString("color: %1; font-size: 20px;").arg(state_color));
    }
    // 更新门状态
    switch (state) {
	case ElevatorState::Open:
	case ElevatorState::Opening:
    case ElevatorState::Closing:
		state_color = "red"; // 门打开或开门中
        break;
    default:
		state_color = "#444"; // 门关闭
		break;
    }
    QLabel* doorLabel = findChild<QLabel*>("doorLabel");
    if (doorLabel) {
        QString doorState = "关闭";
        if (state == ElevatorState::Open) doorState = "打开";
        else if (state == ElevatorState::Opening) doorState = "开门中";
        else if (state == ElevatorState::Closing) doorState = "关门中";

        doorLabel->setText(QString("门状态:       %1").arg(doorState));
        doorLabel->setStyleSheet(QString("color: %1; font-size: 20px;").arg(state_color));
    }

}

void Elevator::ClearAllTimers() {
    if (m_openTimer) {
        m_openTimer->stop();
        delete m_openTimer;
        m_openTimer = nullptr;
    }
    if (m_stayOpenTimer) {
        m_stayOpenTimer->stop();
        delete m_stayOpenTimer;
        m_stayOpenTimer = nullptr;
    }
    if (m_closeTimer) {
        m_closeTimer->stop();
        delete m_closeTimer;
        m_closeTimer = nullptr;
    }
}