#include "RenderWindow.h"

namespace platform
{

// �������� �� ����� ���� ������ ����������,
//  ����� �������� ������� ��������� ��� �������� �����.
constexpr float MAX_ELAPSED_SECONDS = 0.1f;

RenderWindow::RenderWindow(const RenderWindowOptions &options, QWindow *parent)
	: QWindow(parent)
	, m_options(options)
{
	// ������������� ����������� �������.
	setMinimumSize(QSize(options.width, options.height));

	// ���������� OpenGL ��� �������� ����������.
	setSurfaceType(QWindow::OpenGLSurface);

	// ��������� ������ ����� ������� ���������� ���������.
	m_updateTimer.start();
}

RenderWindow::~RenderWindow() = default;

bool RenderWindow::isAnimating() const
{
	return m_isAnimating;
}

void RenderWindow::setAnimating(bool isAnimating)
{
	m_isAnimating = isAnimating;
	if (isAnimating)
	{
		renderLater();
	}
}

void RenderWindow::setScene(std::unique_ptr<IGraphicsScene> scene)
{
	m_scene = std::move(scene);
}

bool RenderWindow::event(QEvent *event)
{
	if (event->type() == QEvent::UpdateRequest) {
		renderNow();
		return true;
	}
	return QWindow::event(event);
}

void RenderWindow::exposeEvent(QExposeEvent *event)
{
	if (isExposed())
	{
		renderNow();
	}
}

void RenderWindow::renderLater()
{
	requestUpdate();
}

void RenderWindow::renderNow()
{
	if (!isExposed())
	{
		return;
	}

	// ������ �������������� �������� OpenGL.
	if (!m_context) {
		m_context = new QOpenGLContext(this);
		m_context->setFormat(requestedFormat());
		m_context->create();
	}

	// ����������� �������� OpenGL � �������� ������ (��� ���������).
	m_context->makeCurrent(this);
	if (m_scene)
	{
		updateScene();
		renderScene();
	}
	// ��������� ����� �������� � ������ ������� ����������� ���������.
	m_context->swapBuffers(this);

	if (m_isAnimating)
	{
		renderLater();
	}
}

void RenderWindow::updateScene()
{
	const float elapsedSeconds = float(m_updateTimer.elapsed()) / 1000.f;

	// ������� ���������� � ������, ���� ������ �� ����� ������ ��������� �����.
	if (elapsedSeconds > 0)
	{
		m_updateTimer.restart();
		m_scene->update((std::min)(elapsedSeconds, MAX_ELAPSED_SECONDS));
	}
}

void RenderWindow::renderScene()
{
	QRect rect(0, 0, width(), height());

	if (!m_device)
	{
		m_device = std::make_unique<QOpenGLPaintDevice>();
	}
	m_device->setSize(size());
	QPainter painter(m_device.get());

	painter.fillRect(0, 0, width(), height(), Qt::white);
	m_scene->redraw(painter);
	painter.end();
}

} // namespace platform
