#if defined(NO_QT5)
#define NO_QT
#endif

#ifndef NO_QT

#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtWidgets/QWidget>
#else
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QWidget>
#endif
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QResizeEvent>
#include <QtGui/QImage>
#include <QIcon>
#include <QProcessEnvironment>

#endif

#define QT_NAME_STRING "Qt5"
#define QT_PLUGIN_ENTRY_NAME gksqt

#include "qtplugin_impl.cxx"

#include "gkswidget.h"


QSize GKSWidget::frame_decoration_size_ = QSize();

static void create_pixmap(ws_state_list *p)
{
  p->pm = new QPixmap(p->width * p->device_pixel_ratio, p->height * p->device_pixel_ratio);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  p->pm->setDevicePixelRatio(p->device_pixel_ratio);
#endif
  p->pm->fill(Qt::white);

  p->pixmap = new QPainter(p->pm);
  p->pixmap->setClipRect(0, 0, p->width, p->height);

  get_pixmap();
}

static void resize_pixmap(int width, int height)
{
  if (p->width != width || p->height != height)
    {
      p->width = width;
      p->height = height;

      if (p->pm)
        {
          delete p->pixmap;
          delete p->pm;

          p->pm = new QPixmap(p->width * p->device_pixel_ratio, p->height * p->device_pixel_ratio);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
          p->pm->setDevicePixelRatio(p->device_pixel_ratio);
#endif
          p->pm->fill(Qt::white);

          p->pixmap = new QPainter(p->pm);
          p->pixmap->setClipRect(0, 0, p->width, p->height);
        }
    }
}

GKSWidget::GKSWidget(QWidget *parent)
    : QWidget(parent), is_mapped(false), resize_requested_by_application(false), dl(NULL)
{
  gkss->fontfile = gks_open_font();

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
  p->device_pixel_ratio = this->devicePixelRatioF();
#elif QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  p->device_pixel_ratio = this->devicePixelRatio();
#else
  p->device_pixel_ratio = 1;
#endif
  p->device_dpi_x = this->physicalDpiX();
  p->device_dpi_y = this->physicalDpiY();
  p->width = 500;
  p->height = 500;
  p->mwidth = (double)p->width / p->device_dpi_x * 0.0254;
  p->mheight = (double)p->height / p->device_dpi_y * 0.0254;
  p->nominal_size = 1.0;

  initialize_data();

  setMinimumSize(2, 2);
  resize(p->width, p->height);
  setWindowTitle(tr("GKS QtTerm"));
  setWindowIcon(QIcon(":/images/gksqt.png"));

  std::string gks_qt_prevent_resize =
      QProcessEnvironment::systemEnvironment().value("GKS_GKSQT_PREVENT_RESIZE").toLower().toStdString();
  if (!gks_qt_prevent_resize.empty())
    {
      p->prevent_resize_by_dl =
          gks_qt_prevent_resize == "1" || gks_qt_prevent_resize == "true" || gks_qt_prevent_resize == "on";
    }
}

GKSWidget::~GKSWidget()
{
  delete[] dl;
}


void GKSWidget::paintEvent(QPaintEvent *)
{
  if (!frame_decoration_size_.isValid() && !(frameGeometry().size() - size()).isNull())
    {
      /* Before the widget is visible `frameGeometry().size()` and `size` are identical
       * -> Only the `paintEvent` can be used reliably to get the window decoration size */
      frame_decoration_size_ = frameGeometry().size() - size();
    }
  if (dl)
    {
      QPainter painter(this);
      p->pm->fill(Qt::white);
      interp(dl);
      painter.drawPixmap(0, 0, *(p->pm));
    }
}

void GKSWidget::resizeEvent(QResizeEvent *event)
{
  p->mwidth = (double)width() / p->device_dpi_x * 0.0254;
  p->mheight = (double)height() / p->device_dpi_y * 0.0254;
  p->nominal_size = min(width(), height()) / 500.0;
  resize_pixmap(nint(width()), nint(height()));
  // Ignore the initial resize event (in this case `width()` and `height()` of the oldSize are `-1`))
  if ((event->oldSize().width() > 0 && event->oldSize().height() > 0) && !resize_requested_by_application)
    {
      p->prevent_resize_by_dl = true;
    }
  resize_requested_by_application = false;
}

void GKSWidget::set_window_size_from_dl()
{
  int sp = 0, *len, *f;
  double *vp;
  len = (int *)(dl + sp);
  while (*len)
    {
      f = (int *)(dl + sp + sizeof(int));
      if (*f == 55)
        {
          vp = (double *)(dl + sp + 3 * sizeof(int));
          p->mwidth = vp[1] - vp[0];
          p->width = nint(p->device_dpi_x * p->mwidth / 0.0254);
          if (p->width < 2)
            {
              p->width = 2;
              p->mwidth = (double)p->width / p->device_dpi_x * 0.0254;
            }

          p->mheight = vp[3] - vp[2];
          p->height = nint(p->device_dpi_y * p->mheight / 0.0254);
          if (p->height < 2)
            {
              p->height = 2;
              p->mheight = (double)p->height / p->device_dpi_y * 0.0254;
            }
          resize_requested_by_application = true;
        }
      sp += *len;
      len = (int *)(dl + sp);
    }
  if (resize_requested_by_application)
    {
      resize(p->width, p->height);
    }
}

void GKSWidget::interpret(char *dl)
{
  delete[] this->dl;
  this->dl = dl;

  if (!p->prevent_resize_by_dl)
    {
      set_window_size_from_dl();
    }
  if (!is_mapped)
    {
      is_mapped = true;
      create_pixmap(p);
      show();
    }

  repaint();
}

void GKSWidget::inqdspsize(double *mwidth, double *mheight, int *width, int *height)
{
  /* forward call to internally included copy of qtplugin_impl.cxx */
  ::inqdspsize(mwidth, mheight, width, height);
}

const QSize &GKSWidget::frame_decoration_size()
{
  return frame_decoration_size_;
}
