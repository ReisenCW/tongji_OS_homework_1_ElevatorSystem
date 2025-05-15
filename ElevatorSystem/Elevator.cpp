#include "Elevator.h"

Elevator::Elevator(int elevator_id, int floor_cnt, QLabel* elevator_floor, QWidget* parent)
    : QWidget(parent), elevator_id(elevator_id), floor_cnt(floor_cnt),
    current_floor(0), state(ElevatorState::Idle), direction(Direction::None),
	elevator_floor(elevator_floor), is_door_open(false), is_alarm_active(false)
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

void Elevator::InitParams()
{
    current_floor = 0;
    state = ElevatorState::Idle;
    direction = Direction::None;
    internal_targets.clear();
    external_up_requests.clear();
    external_down_requests.clear();
}

void Elevator::InitWidget()
{
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

    QLabel* titleLabel = new QLabel(QString("电梯 %1").arg(elevator_id), mainContainer);
    titleLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: #333;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QHBoxLayout* bodyLayout = new QHBoxLayout();
    mainLayout->addLayout(bodyLayout, 1);

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

    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(10);
    bodyLayout->addLayout(rightLayout, 1);

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
            "   background-color: white;"
            "}"
            "QPushButton:disabled {"
            "   background-color: red;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #ccc;"
            "   border: 1px solid #999;"
            "   color: #fff;"
            "}"
        );
        connect(btn, &QPushButton::clicked, this, [=]() {
            this->AddInternalTarget(i);
            });
        floorButtons.push_back(btn);
        btnGrid->addWidget(btn, i / BUTTONS_PER_ROW, i % BUTTONS_PER_ROW);
    }
    rightLayout->addLayout(btnGrid);

    QHBoxLayout* doorLayout = new QHBoxLayout();
    QPushButton* openBtn = CreateDoorButton("开门", "lightgreen");
    QPushButton* closeBtn = CreateDoorButton("关门", "lightcoral");
    connect(openBtn, &QPushButton::clicked, this, &Elevator::HandleOpenDoor);
    connect(closeBtn, &QPushButton::clicked, this, &Elevator::HandleCloseDoor);
    doorLayout->addWidget(openBtn);
    doorLayout->addWidget(closeBtn);

    // 新增报警按钮
    QPushButton* alarmBtn = new QPushButton("报警", this);
    alarmBtn->setFixedSize(50, 20);
    alarmBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: yellow;"
        "   border: 1px solid #666;"
        "   border-radius: 5px;"
        "}"
        "QPushButton:pressed { background-color: #ffd700; }"
    );
    connect(alarmBtn, &QPushButton::clicked, this, &Elevator::HandleAlarm);
    doorLayout->addWidget(alarmBtn);

    rightLayout->addLayout(doorLayout);

    QGroupBox* statusGroup = CreateStatusGroup();
    rightLayout->addWidget(statusGroup);

    QVBoxLayout* windowLayout = new QVBoxLayout(this);
    windowLayout->setContentsMargins(0, 0, 0, 0);
    windowLayout->addWidget(mainContainer);
    this->setLayout(windowLayout);
}

void Elevator::AddInternalTarget(int floor)
{
    if (floor == current_floor) return;
    if (!internal_targets.insert(floor).second) return;
    if (floor >= 0 && floor < floorButtons.size()) {
        floorButtons[floor]->setDisabled(true);
    }
    if (state == ElevatorState::Idle) {
        DecideNextAction();
    }
}

void Elevator::AddExternalRequest(int floor, Direction dir)
{
    if (dir == Direction::Up) {
        if (!external_up_requests.insert(floor).second) return;
    }
    else if (dir == Direction::Down) {
        if (!external_down_requests.insert(floor).second) return;
    }
    if (state == ElevatorState::Idle) {
        DecideNextAction();
    }
}

bool Elevator::InternalRequestExists(int floor) const
{
    return internal_targets.count(floor) > 0;
}

bool Elevator::ExternalRequestExists(int floor, Direction dir) const
{
    if (dir == Direction::Up)
        return external_up_requests.count(floor) > 0;
    else if (dir == Direction::Down)
        return external_down_requests.count(floor) > 0;
    return false;
}

void Elevator::ClearAllTimers()
{
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

void Elevator::DecideNextAction()
{
    // 选择方向
    if (!internal_targets.empty() || !external_up_requests.empty() || !external_down_requests.empty()) {
        // 优先上行
        int up_min = INT_MAX, down_max = INT_MIN;
        for (int f : internal_targets) {
            if (f > current_floor) up_min = std::min(up_min, f);
            if (f < current_floor) down_max = std::max(down_max, f);
        }
        // 处理外部上升请求和外部下降请求中高于当前楼层的楼层
        for (int f : external_up_requests) {
            if (f > current_floor) up_min = std::min(up_min, f);
        }
        // 处理外部下降请求中的上行目标
        for (int f : external_down_requests) {  
            if (f > current_floor) up_min = std::min(up_min, f);
        }
        // 处理外部下降请求和外部上升请求中低于当前楼层的楼层
        for (int f : external_down_requests) {
            if (f < current_floor) down_max = std::max(down_max, f);
        }
        // 处理外部上升请求中的下行目标
        for (int f : external_up_requests) {    
            if (f < current_floor) down_max = std::max(down_max, f);
        }
        if (up_min != INT_MAX) {
            direction = Direction::Up;
            state = ElevatorState::Up;
        }
        else if (down_max != INT_MIN) {
            direction = Direction::Down;
            state = ElevatorState::Down;
        }
        else if (!internal_targets.empty()) {
            // 只剩本层目标
            direction = Direction::None;
            state = ElevatorState::Idle;
        }
        else {
            direction = Direction::None;
            state = ElevatorState::Idle;
        }
		qDebug() << "up_min:" << up_min << "down_max:" << down_max;
        QTimer::singleShot(600, this, &Elevator::MoveToNextFloor);
    }
    else {
        direction = Direction::None;
        state = ElevatorState::Idle;
        UpdateDisplay();
    }
}

void Elevator::MoveToNextFloor()
{
    if (state == ElevatorState::Opening || state == ElevatorState::Open || state == ElevatorState::Closing)
        return;

    // 1. 查找当前方向上的下一个目标
    int next = -1;
    if (direction == Direction::Up) {
        int min_above = INT_MAX;
        for (int f : internal_targets)
            if (f > current_floor) 
                min_above = std::min(min_above, f);
        for (int f : external_up_requests)
            if (f > current_floor) 
                min_above = std::min(min_above, f);
        // 如果没有,再从external_down_requests中找
        if (min_above == INT_MAX) {
            for (int f : external_down_requests) {
                min_above = std::min(min_above, f);
            }
        }

        if (min_above != INT_MAX) next = min_above;
    }
    else if (direction == Direction::Down) {
        int max_below = INT_MIN;
        for (int f : internal_targets)
            if (f < current_floor) 
                max_below = std::max(max_below, f);
        for (int f : external_down_requests)
            if (f < current_floor) 
                max_below = std::max(max_below, f);
		// 如果没有,再从external_up_requests中找
		if (max_below == INT_MIN) {
			for (int f : external_up_requests) {
				max_below = std::max(max_below, f);
			}
		}

        if (max_below != INT_MIN) next = max_below;
    }

    // 2. 没有目标则换向或Idle
    if (next == -1) {
        // 尝试换向
        if (direction == Direction::Up) {
            int max_below = INT_MIN;
            for (int f : internal_targets)
                if (f < current_floor) max_below = std::max(max_below, f);
            for (int f : external_down_requests)
                if (f < current_floor) max_below = std::max(max_below, f);
            if (max_below != INT_MIN) {
                direction = Direction::Down;
                state = ElevatorState::Down;
                QTimer::singleShot(600, this, &Elevator::MoveToNextFloor);
                return;
            }
        }
        else if (direction == Direction::Down) {
            int min_above = INT_MAX;
            for (int f : internal_targets)
                if (f > current_floor) min_above = std::min(min_above, f);
            for (int f : external_up_requests)
                if (f > current_floor) min_above = std::min(min_above, f);
            if (min_above != INT_MAX) {
                direction = Direction::Up;
                state = ElevatorState::Up;
                QTimer::singleShot(600, this, &Elevator::MoveToNextFloor);
                return;
            }
        }
        // 没有目标，Idle
        direction = Direction::None;
        state = ElevatorState::Idle;
        UpdateDisplay();
        return;
    }
    qDebug() << "next:" << next;

    // 3. 移动一层
    if (next > current_floor) {
        current_floor++;
        state = ElevatorState::Up;
    }
    else if (next < current_floor) {
        current_floor--;
        state = ElevatorState::Down;
    }

    // 4. 到达目标楼层，处理开门、请求清除
    bool stop = false;
    if (internal_targets.count(current_floor)) {
        internal_targets.erase(current_floor);
        stop = true;
    }
    if (external_up_requests.count(current_floor)) {
        external_up_requests.erase(current_floor);
        stop = true;
    }
    if (external_down_requests.count(current_floor)) {
        external_down_requests.erase(current_floor);
        stop = true;
    }

    if (stop) {
        if (current_floor >= 0 && current_floor < floorButtons.size()) {
            floorButtons[current_floor]->setDisabled(false);
        }
        emit FloorArrived(current_floor, state);
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
                    DecideNextAction();
                    });
                });
            });
    }
    else {
        QTimer::singleShot(600, this, &Elevator::MoveToNextFloor);
    }

    elevator_floor->setText(QString::number(current_floor + 1));
    elevator_floor->repaint();
    UpdateDisplay();
}

void Elevator::HandleOpenDoor()
{
    ClearAllTimers();
    if (state == ElevatorState::Idle ||
        state == ElevatorState::Closing ||
        state == ElevatorState::Open)
    {
        state = ElevatorState::Opening;
        UpdateDisplay();
        int arrived_floor = current_floor;
        if (arrived_floor >= 0 && arrived_floor < floorButtons.size()) {
            QPushButton* btn = floorButtons[arrived_floor];
            btn->setDisabled(false);
        }
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
                    DecideNextAction();
                });
            });
        });
    }
}

void Elevator::HandleCloseDoor()
{
    ClearAllTimers();
    if (state == ElevatorState::Open || state == ElevatorState::Opening)
    {
        state = ElevatorState::Closing;
        UpdateDisplay();
        m_closeTimer = new QTimer(this);
        m_closeTimer->singleShot(1000, [=]() {
            DecideNextAction();
        });
    }
}

void Elevator::HandleAlarm() {
    ClearAllTimers();
    if (!is_alarm_active) {
        is_alarm_active = true;
		state = ElevatorState::Warning;
        UpdateDisplay();
        emit AlarmTriggered(elevator_id); // 触发报警信号

        // 3秒后复位报警状态
        QTimer::singleShot(3000, [=]() {
            is_alarm_active = false;
			state = ElevatorState::Idle;
            UpdateDisplay();
            DecideNextAction();
        });
    }
}

QPushButton* Elevator::CreateDoorButton(const QString& text, const QString& color)
{
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

QGroupBox* Elevator::CreateStatusGroup()
{
    QGroupBox* group = new QGroupBox("电梯状态", this);
    group->setStyleSheet("QGroupBox::title { text-align: center; font-size: 24px;}");
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setSpacing(5);

    const QList<QPair<QString, QString>> statusLabels = {
        {"状态:", "           空闲"}, {"当前楼层:", "   0"}, {"门状态:", "       关闭"}
    };

    for (const auto& pair : statusLabels) {
        QLabel* label = new QLabel(pair.first + pair.second, group);
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

void Elevator::UpdateDisplay()
{
    QSlider* floorSlider = findChild<QSlider*>();
    if (floorSlider) {
        floorSlider->setValue(current_floor + 1);
    }
    QLabel* floorValueLabel = findChild<QLabel*>("floorValueLabel");
    if (floorValueLabel) {
        floorValueLabel->setText(QString::number(current_floor + 1));
    }
    // 状态栏
    QString state_color = "#444";
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
		case ElevatorState::Warning: state_str = "报警中"; break;
        }
        stateLabel->setText(QString("状态:          %1").arg(state_str));
        stateLabel->setStyleSheet(QString("color: %1; font-size: 20px;").arg(state_color));
    }
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
    switch (state) {
    case ElevatorState::Open:
    case ElevatorState::Opening:
    case ElevatorState::Closing:
        state_color = "red";
        break;
    default:
        state_color = "#444";
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
