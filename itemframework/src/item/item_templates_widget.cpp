#include "item_templates_widget.h"
#include "ui_item_templates_widget.h"

#include "item_templates_model.h"

#include <QGraphicsItem>
#include <QInputDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <cmath>

ItemTemplatesWidget::ItemTemplatesWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ItemTemplatesWidget),
    _model{new ItemTemplatesModel{this}}
{
    ui->setupUi(this);
    ui->templatesView->setModel(_model);
    ui->templatesView->installEventFilter(this);
    ui->templatePreview->setDragMode(QGraphicsView::ScrollHandDrag);

    auto* const previewScene = new QGraphicsScene(this);
    ui->templatePreview->setScene(previewScene);

    ui->templatesView->setMaximumHeight(25*4); // roughly enough space for four entries

    connect(ui->templatesView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &ItemTemplatesWidget::handleSelectionChanged);

    connect(ui->deleteButton, &QPushButton::pressed, [this]() {
        deleteTemplates(this->ui->templatesView->selectionModel()->selectedIndexes());
    });

    connect(ui->enablePreview, &QCheckBox::toggled, [this](bool checked) {
        bool const enablePreview = checked;
        this->ui->templatePreview->setVisible(enablePreview);
        this->ui->zoomLabel->setVisible(enablePreview);
        this->ui->zoomSlider->setVisible(enablePreview);
    });

    connect(ui->zoomSlider, &QSlider::valueChanged, [this](int value) {
        auto const scalingFactor = static_cast<float>(value) / _sliderValuesFactor;
        this->ui->templatePreview->setTransform(QTransform::fromScale(scalingFactor, scalingFactor));
    });
}

ItemTemplatesWidget::~ItemTemplatesWidget()
{
    delete ui;
}

void ItemTemplatesWidget::createTemplate(QDomDocument const& items, QPixmap const& pixmap)
{
    QString const name = promptUserForTemplateName();
    if (!name.isEmpty()) {
        saveTemplate(name, items, pixmap);
    }
}

QString ItemTemplatesWidget::promptUserForTemplateName()
{
    QString name{};
    bool shouldCancel{false};

    do {
        name = askTemplateName(&shouldCancel);
    } while (name.isNull() && !shouldCancel);

    return name;
}

QString ItemTemplatesWidget::askTemplateName(bool* shouldCancel)
{
    QString name{};
    QInputDialog createTemplateDialog{};
    createTemplateDialog.setInputMode(QInputDialog::TextInput);
    createTemplateDialog.setLabelText(tr("Please enter a name for this template:"));

    int const result = createTemplateDialog.exec();
    switch (result) {
        case QDialog::Accepted: {
            QString const input = createTemplateDialog.textValue();
            if (_model->isDuplicate(input)) {
                QMessageBox::warning(this, tr("Duplicate template name"), tr("A template with this name already exists"));
            } else {
                name = input;
            }

            break;
        }

        case QDialog::Rejected:
            if (shouldCancel != nullptr) {
                *shouldCancel = true;
            }

            break;

        default:
            break;
    }

    return name;
}

void ItemTemplatesWidget::saveTemplate(QString const& name, QDomDocument const& items, QPixmap const& pixmap)
{
    auto* const item = new QStandardItem{name};
    item->setData(items.toByteArray(), Qt::UserRole);
    item->setData(pixmap, Qt::UserRole + 1);

    _model->appendRow(item);
}

void ItemTemplatesWidget::deleteTemplates(QModelIndexList const& indexes)
{
    auto const response = QMessageBox::question(this, "Confirm", "Are you sure you want to delete the selected templates?");
    if (response == QMessageBox::Yes) {
        QVector<QPersistentModelIndex> persistentIndexes(indexes.count());
        std::copy(indexes.cbegin(), indexes.cend(), persistentIndexes.begin());

        for (auto index : persistentIndexes) {
            _model->removeRow(index.row());
        }
    }
}

bool ItemTemplatesWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        auto const key = static_cast<QKeyEvent*>(event)->key();
        if (key == Qt::Key_Delete && obj == ui->templatesView) {
            deleteTemplates(ui->templatesView->selectionModel()->selectedIndexes());

            return true;
        }
    }

    return false;
}

void ItemTemplatesWidget::handleSelectionChanged(QModelIndex const& current, QModelIndex const&)
{
    ui->deleteButton->setEnabled(true);
    ui->templatePreview->setEnabled(true);

    updatePreview(current);
    updateZoom();
}

void ItemTemplatesWidget::updatePreview(QModelIndex const& current)
{
    auto previewScene = ui->templatePreview->scene();
    auto previewPixmap = _model->data(current, Qt::UserRole + 1).value<QPixmap>();
    auto previewImage = previewPixmap.toImage();

    previewScene->clear();
    previewScene->addPixmap(previewPixmap);
    previewScene->setSceneRect(previewImage.rect());
}

void ItemTemplatesWidget::updateZoom()
{
    auto* const previewScene = ui->templatePreview->scene();

    float const sx = this->ui->templatePreview->viewport()->geometry().width() / previewScene->width();
    float const sy = this->ui->templatePreview->viewport()->geometry().height() / previewScene->height();
    float const scalingFactor = qMin(sx, sy);

    int const sliderMin = ceil(_sliderValuesFactor * scalingFactor);
    int const sliderMax = _sliderValuesFactor * 1;

    ui->zoomSlider->setRange(sliderMin, sliderMax);
    ui->zoomSlider->setValue(sliderMin);
    ui->templatePreview->setTransform(QTransform::fromScale(scalingFactor, scalingFactor));
}
