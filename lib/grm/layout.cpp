#include <iostream>
#include <limits>
#include <sstream>
#include "layout.hpp"

double epsilon = std::numeric_limits<double>::epsilon();

Slice::Slice(int rowStart, int rowStop, int colStart, int colStop)
    : rowStart(rowStart), rowStop(rowStop), colStart(colStart), colStop(colStop)
{
}

Slice *Slice::copy()
{
  Slice *copy = new Slice(this->rowStart, this->rowStop, this->colStart, this->colStop);
  return copy;
}

GridElement::GridElement()
{
  subplot = new double[4];
};

void GridElement::setSubplot(double x1, double x2, double y1, double y2)
{
  if (finalized || subplotSet == 0)
    {
      subplot[0] = x1;
      subplot[1] = x2;
      subplot[2] = y1;
      subplot[3] = y2;

      finalized = 0;
      subplotSet = 1;
    }
  else
    {
      if (y1 < subplot[2])
        {
          subplot[2] = y1;
        }
      if (x2 > subplot[1])
        {
          subplot[1] = x2;
        }
    }
}

void GridElement::setAbsHeight(double height)
{
  if (heightSet)
    {
      throw std::invalid_argument("Can only set one height attribute");
    }
  if (height <= 0 || height > 1)
    {
      throw std::invalid_argument("Height has to be between 0 and 1");
    }
  if (arSet and widthSet)
    {
      throw std::invalid_argument("You cant restrict the height on a plot with fixed width and aspect ratio");
    }
  absHeight = height;
  heightSet = 1;
}

void GridElement::setAbsHeightPxl(int height)
{
  if (heightSet)
    {
      throw std::invalid_argument("Can only set one height attribute");
    }
  if (height <= 0)
    {
      throw std::invalid_argument("Pixel height has to be an positive integer");
    }
  if (arSet and widthSet)
    {
      throw std::invalid_argument("You cant restrict the height on a plot with fixed width and aspect ratio");
    }
  absHeightPxl = height;
  heightSet = 1;
}

void GridElement::setRelativeHeight(double height)
{
  if (heightSet)
    {
      throw std::invalid_argument("Can only set one height attribute");
    }
  if (height <= 0 || height > 1)
    {
      throw std::invalid_argument("Height has to be between 0 and 1");
    }
  if (arSet and widthSet)
    {
      throw std::invalid_argument("You cant restrict the height on a plot with fixed width and aspect ratio");
    }
  relativeHeight = height;
  heightSet = 1;
}

void GridElement::setAbsWidth(double width)
{
  if (widthSet)
    {
      throw std::invalid_argument("Can only set one width attribute");
    }
  if (width <= 0 || width > 1)
    {
      throw std::invalid_argument("Width has to be between 0 and 1");
    }
  if (arSet and heightSet)
    {
      throw std::invalid_argument("You cant restrict the width on a plot with fixed height and aspect ratio");
    }
  absWidth = width;
  widthSet = 1;
}

void GridElement::setAbsWidthPxl(int width)
{
  if (widthSet)
    {
      throw std::invalid_argument("Can only set one width attribute");
    }
  if (width <= 0)
    {
      throw std::invalid_argument("Pixel Width has to be an positive integer");
    }
  if (arSet and heightSet)
    {
      throw std::invalid_argument("You cant restrict the width on a plot with fixed height and aspect ratio");
    }
  absWidthPxl = width;
  widthSet = 1;
}

void GridElement::setRelativeWidth(double width)
{
  if (widthSet)
    {
      throw std::invalid_argument("Can only set one width attribute");
    }
  if (width <= 0 || width > 1)
    {
      throw std::invalid_argument("Width has to be between 0 and 1");
    }
  if (arSet and heightSet)
    {
      throw std::invalid_argument("You cant restrict the width on a plot with fixed height and aspect ratio");
    }
  relativeWidth = width;
  widthSet = 1;
}

void GridElement::setAspectRatio(double ar)
{
  if (ar <= 0)
    {
      throw std::invalid_argument("Aspect ration has to be bigger than 0");
    }
  if (widthSet && heightSet)
    {
      throw std::invalid_argument("You cant restrict the aspect ratio on a plot with fixed sides");
    }
  aspectRatio = ar;
  arSet = 1;
}

void GridElement::finalizeSubplot()
{
  if (finalized)
    {
      return;
    }

  if (absHeight != -1)
    {
      double availableHeight = subplot[3] - subplot[2];
      if (absHeight > availableHeight + epsilon)
        {
          throw std::invalid_argument("Absolute height is bigger than available height");
        }
      double middle = subplot[2] + availableHeight / 2;
      subplot[2] = middle - absHeight / 2;
      subplot[3] = middle + absHeight / 2;
    }

  if (absWidth != -1)
    {
      double availableWidth = subplot[1] - subplot[0];
      if (absWidth > availableWidth + epsilon)
        {
          throw std::invalid_argument("Absolute width is bigger than available width");
        }
      double middle = subplot[0] + availableWidth / 2;
      subplot[0] = middle - absWidth / 2;
      subplot[1] = middle + absWidth / 2;
    }

  if (relativeHeight != -1)
    {
      double availableHeight = subplot[3] - subplot[2];
      double middle = subplot[2] + availableHeight / 2;
      double newHeight = availableHeight * relativeHeight;
      subplot[2] = middle - newHeight / 2;
      subplot[3] = middle + newHeight / 2;
    }

  if (relativeWidth != -1)
    {
      double availableWidth = subplot[1] - subplot[0];
      double middle = subplot[0] + availableWidth / 2;
      double newWidth = availableWidth * relativeWidth;
      subplot[0] = middle - newWidth / 2;
      subplot[1] = middle + newWidth / 2;
    }

  /* TODO: implement for pxl */

  if (arSet)
    {
      double currentHeigth = (subplot[3] - subplot[2]);
      double currentWidth = (subplot[1] - subplot[0]);
      double currentAR = currentWidth / currentHeigth;

      if (currentAR < aspectRatio)
        {
          double newHeight = currentWidth / aspectRatio;
          double middle = subplot[2] + currentHeigth / 2;
          subplot[2] = middle - newHeight / 2;
          subplot[3] = middle + newHeight / 2;
        }
      else
        {
          double newWidth = currentHeigth * aspectRatio;
          double middle = subplot[0] + currentWidth / 2;
          subplot[0] = middle - newWidth;
          subplot[1] = middle + newWidth;
        }
    }

  if (subplot_args != nullptr)
    {
      grm_args_push(subplot_args, "subplot", "nD", 4, subplot);
    }

  finalized = 1;
}

double *GridElement::getSubplot()
{
  return subplot;
}

void GridElement::setFitParentsHeight(bool fitParentsHeight)
{
  this->fitParentsHeight = fitParentsHeight;
}

void GridElement::setFitParentsWidth(bool fitParentsWidth)
{
  this->fitParentsWidth = fitParentsWidth;
}

Grid::Grid(int nrows, int ncols) : GridElement(), nrows(nrows), ncols(ncols)
{
  int i;
  for (i = 0; i < nrows; ++i)
    {
      std::vector<GridElement *> row(ncols, nullptr);
      rows.push_back(row);
    }
}

Grid::~Grid()
{
  for (auto const &x : this->elementToPosition)
    {
      /* delete allocated slices */
      delete x.second;
    }
}

GridElement *Grid::getElement(int row, int col) const
{
  return rows.at(row).at(col);
};

void Grid::setElement(int row, int col, GridElement *element)
{
  int nrowsToAllocate, ncolsToAllocate;
  Slice *oldSlice;

  nrowsToAllocate = row + 1;
  ncolsToAllocate = col + 1;

  /* Resize the container if necessary */
  upsize(nrowsToAllocate, ncolsToAllocate);

  /* Delete element from grid if it already exists */
  try
    {
      oldSlice = elementToPosition.at(element);
      rows.at(oldSlice->rowStart).at(oldSlice->colStart) = nullptr;
      elementToPosition.erase(element);
      delete (oldSlice);
    }
  catch (const std::out_of_range &e)
    {
    };

  rows.at(row).at(col) = element;
  Slice *newSlice = new Slice(row, row, col, col);
  elementToPosition[element] = newSlice;
}

void Grid::setElement(int row, int col, grm_args_t *subplot_args)
{
  GridElement *element = nullptr;
  const char *grid_element_address = nullptr;
  if (args_values(subplot_args, "grid_element", "s", &grid_element_address))
    {
      element = reinterpret_cast<GridElement *>(std::stoi(grid_element_address));
    }
  else
    {
      element = new GridElement();
      element->subplot_args = subplot_args;
    }
  setElement(row, col, element);
  std::stringstream address_stream;
  address_stream << element;
  grm_args_push(subplot_args, "grid_element", "s", address_stream.str().c_str());
}

void Grid::setElement(Slice *slice, GridElement *element)
{
  int nrowsToAllocate, ncolsToAllocate;
  Slice *oldSlice;

  nrowsToAllocate = slice->rowStop;
  ncolsToAllocate = slice->colStop;

  /* Resize the container if necessary */
  upsize(nrowsToAllocate, ncolsToAllocate);

  /* Delete element from grid if it already exists */
  try
    {
      oldSlice = elementToPosition.at(element);
      rows.at(oldSlice->rowStart).at(oldSlice->colStart) = nullptr;
      elementToPosition.erase(element);
      delete (oldSlice);
    }
  catch (const std::out_of_range &e)
    {
    };

  for (int row = slice->rowStart; row < slice->rowStop; ++row)
    {
      for (int col = slice->colStart; col < slice->colStop; ++col)
        {
          rows.at(row).at(col) = element;
        }
    }
  Slice *newSlice = slice->copy();
  elementToPosition[element] = newSlice;
}

void Grid::setElement(Slice *slice, grm_args_t *subplot_args)
{
  GridElement *element = nullptr;
  const char *grid_element_address = nullptr;
  if (args_values(subplot_args, "grid_element", "s", &grid_element_address))
    {
      element = reinterpret_cast<GridElement *>(std::stoi(grid_element_address));
    }
  else
    {
      element = new GridElement();
      element->subplot_args = subplot_args;
    }
  setElement(slice, element);
  std::stringstream address_stream;
  address_stream << element;
  grm_args_push(subplot_args, "grid_element", "s", address_stream.str().c_str());
}

void Grid::printGrid() const
{
  double *subplot;
  for (int i = 0; i < nrows; i++)
    {
      for (int j = 0; j < ncols; j++)
        {
          subplot = getElement(i, j)->subplot;
          printf("[%f %f %f %f] ", subplot[0], subplot[1], subplot[2], subplot[3]);
        }
      printf("\n");
    }
}

void Grid::finalizeSubplot()
{
  double xmin, xmax, ymin, ymax, rowHeight, elementWidth;
  int y, x, rowSpan, colSpan;
  GridElement *element;

  if (!subplotSet)
    {
      setSubplot(0, 1, 0, 1);
    }

  GridElement::finalizeSubplot();

  /* calculate height of each row */
  double rowHeights[nrows];
  double totalHeightLeft = subplot[3] - subplot[2];
  int numRowsWithFlexibleHeight = 0;
  for (y = 0; y < nrows; y++)
    {
      double rowHeight = -1;
      for (x = 0; x < ncols; x++)
        {
          element = getElement(y, x);
          if (element != nullptr && element->fitParentsHeight && element->absHeight != -1)
            {
              /* taking into account that an element can range over multiple rows */
              rowSpan = this->getRowSpan(element);
              if (element->absHeight / rowSpan > rowHeight)
                {
                  rowHeight = element->absHeight / rowSpan;
                }
            }
        }
      rowHeights[y] = rowHeight;
      if (rowHeight == -1)
        {
          numRowsWithFlexibleHeight += 1;
        }
      else
        {
          totalHeightLeft -= rowHeight;
        }
    }
  if (totalHeightLeft + epsilon < 0)
    {
      throw std::invalid_argument("Not enough vertical space for the rows");
    }
  if (numRowsWithFlexibleHeight == 0)
    {
      /* distribute the height that is left */
      for (y = 0; y < nrows; y++)
        {
          rowHeights[y] += totalHeightLeft / nrows;
        }
    }

  /* calculate width of each column */
  double totalWidthLeft = subplot[1] - subplot[0];
  double colWidths[ncols];
  int numColsWithFlexibleWidth = 0;
  for (x = 0; x < ncols; x++)
    {
      double colWidth = -1;
      for (y = 0; y < nrows; y++)
        {
          element = getElement(y, x);
          if (element != nullptr && element->fitParentsWidth && element->absWidth != -1)
            {
              /* taking into account that an element can range over multiple columns */
              colSpan = this->getColSpan(element);
              if (element->absWidth / colSpan > colWidth)
                {
                  colWidth = element->absWidth / colSpan;
                }
            }
        }
      colWidths[x] = colWidth;
      if (colWidth == -1)
        {
          numColsWithFlexibleWidth += 1;
        }
      else
        {
          totalWidthLeft -= colWidth;
        }
    }
  if (totalWidthLeft + epsilon < 0)
    {
      throw std::invalid_argument("Not enough horizontal space for the cols");
    }
  if (numColsWithFlexibleWidth == 0)
    {
      for (x = 0; x < ncols; x++)
        {
          colWidths[x] += totalWidthLeft / ncols;
        }
    }

  /* calculate the subplot for each element */
  ymax = subplot[3];
  ymin = ymax;
  for (y = 0; y < nrows; y++)
    {
      xmin = subplot[0];
      xmax = xmin;

      rowHeight = (rowHeights[y] == -1) ? totalHeightLeft / numRowsWithFlexibleHeight : rowHeights[y];
      ymin -= rowHeight;

      for (x = 0; x < ncols; x++)
        {
          element = getElement(y, x);

          elementWidth = (colWidths[x] == -1) ? totalWidthLeft / numColsWithFlexibleWidth : colWidths[x];
          xmax += elementWidth;

          if (element != nullptr)
            {
              element->setSubplot(xmin, xmax, ymin, ymax);
            }
          xmin = xmax;
        }
      ymax = ymin;
    }

  /* call finalize on each element */
  for (y = 0; y < nrows; y++)
    {
      for (x = 0; x < ncols; x++)
        {
          element = getElement(y, x);
          if (element != nullptr)
            {
              element->finalizeSubplot();
            }
        }
    }
}

void Grid::upsize(int nrows, int ncols)
{
  int i;
  if (ncols > this->ncols)
    {
      for (i = 0; i < this->nrows; ++i)
        {
          rows.at(i).resize(ncols, nullptr);
        }
      this->ncols = ncols;
    }
  if (nrows > this->nrows)
    {
      for (i = this->nrows; i < nrows; ++i)
        {
          std::vector<GridElement *> row(this->ncols, nullptr);
          this->rows.insert(this->rows.end(), row);
        }
      this->nrows = nrows;
    }
}

void Grid::trim()
{
  int row, col;
  bool removeRow, removeCol;
  std::vector<int> colsToRemove;

  /* remove empty rows */
  auto rowIterator = rows.begin();
  while (rowIterator != rows.end())
    {
      removeRow = true;
      for (auto colIterator = rowIterator->begin(); colIterator != rowIterator->end(); ++colIterator)
        {
          if (*colIterator != nullptr)
            {
              removeRow = false;
            }
        }
      if (removeRow)
        {
          rowIterator = rows.erase(rowIterator);
          --(nrows);
        }
      else
        {
          ++rowIterator;
        }
    }

  /* remove empty cols */
  col = 0;
  while (col != ncols)
    {
      removeCol = true;
      for (row = 0; row < nrows; ++row)
        {
          if (getElement(row, col) != nullptr)
            {
              removeCol = false;
            }
        }
      if (removeCol)
        {
          for (row = 0; row < nrows; ++row)
            {
              auto colIterator = rows.at(row).begin();
              rows.at(row).erase(colIterator + col);
            }
          --(ncols);
        }
      else
        {
          ++col;
        }
    }
}

int Grid::getColSpan(GridElement *element)
{
  Slice *slice = elementToPosition.at(element);
  return slice->colStop - slice->colStart;
}

int Grid::getRowSpan(GridElement *element)
{
  Slice *slice = elementToPosition.at(element);
  return slice->rowStop - slice->rowStart;
}
