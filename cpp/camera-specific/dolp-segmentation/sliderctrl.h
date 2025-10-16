#pragma once
/*
Prompt:
C++ Qt user control, which has a groupbox with title, a horizontal layout with slider and label. 
The label shows the values of the slider. The constructor receives titel, default value, min and max. 
The slider change events are forwarded to the parent.
*/


#include <QWidget>
#include <QGroupBox>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QEvent>
#include <cmath>


class SliderControl : public QWidget {
    Q_OBJECT

public:
    // Constructor
    explicit SliderControl(const QString& title, int defaultValue, int minValue, int maxValue, QWidget* parent = nullptr)
        : QWidget(parent), m_minValue(minValue), m_maxValue(maxValue) {

        // Create a group box with title
        QGroupBox* groupBox = new QGroupBox(title, this);

        // Create the slider
        m_slider = new QSlider(Qt::Horizontal, groupBox);
        m_slider->setRange(minValue, maxValue);
        m_slider->setValue(defaultValue);

        // Create the label to show the slider's value
        m_valueLabel = new QLabel(QString::number(defaultValue), groupBox);

        // Connect the slider value change signal to the update label slot
        connect(m_slider, &QSlider::valueChanged, this, &SliderControl::onSliderValueChanged);

        // Create a horizontal layout for slider and label
        QHBoxLayout* hLayout = new QHBoxLayout;
        hLayout->addWidget(m_slider);
        hLayout->addWidget(m_valueLabel);

        // Set the layout to the group box
        groupBox->setLayout(hLayout);

        // Main layout for this control (Optional: you can add other controls)
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(groupBox);

        setLayout(mainLayout);
    }

signals:
    void sliderValueChanged(int value);

private slots:
    // Slot to update the label when the slider value changes
    void onSliderValueChanged(int value) {
        m_valueLabel->setText(QString::number(value));
        emit sliderValueChanged(value); // Forward the change to the parent
    }

private:
    int m_minValue;
    int m_maxValue;
    QSlider* m_slider;
    QLabel* m_valueLabel;
};


class LogSliderControl : public QWidget {
    Q_OBJECT

public:
    // Constructor
    explicit LogSliderControl(const QString& title, double defaultValue, double minValue, double maxValue, QWidget* parent = nullptr)
        : QWidget(parent), m_minValue(minValue), m_maxValue(maxValue) {

        // Create a group box with title
        QGroupBox* groupBox = new QGroupBox(title, this);
        m_minValue = std::max(minValue, 0.0001);
        // Create the slider with a linear range, we'll map it to logarithmic scale
        m_slider = new QSlider(Qt::Horizontal, groupBox);
        m_slider->setRange(0, 100); // Range of the slider (0 to 100, for simplicity)

        // Map the default value to the logarithmic scale range
        int defaultSliderValue = mapToSlider(defaultValue);
        m_slider->setValue(defaultSliderValue);

        // Create the label to show the slider's value
        m_valueLabel = new QLabel(QString::number(defaultValue), groupBox);

        // Connect the slider value change signal to the update label slot
        connect(m_slider, &QSlider::valueChanged, this, &LogSliderControl::onSliderValueChanged);

        // Create a horizontal layout for slider and label
        QHBoxLayout* hLayout = new QHBoxLayout;
        hLayout->addWidget(m_slider);
        hLayout->addWidget(m_valueLabel);

        // Set the layout to the group box
        groupBox->setLayout(hLayout);

        // Main layout for this control (Optional: you can add other controls)
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(groupBox);

        setLayout(mainLayout);
    }

signals:
    void sliderValueChanged(double value);

private slots:
    // Slot to update the label when the slider value changes
    void onSliderValueChanged(int sliderValue) {
        double value = mapToLogarithmic(sliderValue); // Convert slider value to log scale value
        m_valueLabel->setText(QString::number(value));
        emit sliderValueChanged(value); // Forward the change to the parent
    }

private:
    // Mapping linear slider value to logarithmic value
    double mapToLogarithmic(int sliderValue) const {
        // Convert slider value to logarithmic scale
        double range = m_maxValue - m_minValue;
        double logRange = log10(m_maxValue) - log10(m_minValue);  // Logarithmic scale range
        double scale = sliderValue / 100.0;  // Slider scale is between 0 and 100
        double logValue = pow(10, log10(m_minValue) + scale * logRange);  // Mapping to log scale
        return logValue;
    }

    // Mapping logarithmic value to the slider value
    int mapToSlider(double logValue) const {
        double range = m_maxValue - m_minValue;
        double logRange = log10(m_maxValue) - log10(m_minValue);
        double logValueNorm = log10(logValue) - log10(m_minValue);
        double scale = logValueNorm / logRange;
        return static_cast<int>(scale * 100);
    }

private:
    double m_minValue;
    double m_maxValue;
    QSlider* m_slider;
    QLabel* m_valueLabel;
};

