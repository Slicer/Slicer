#ifndef __PrettyPrintTable_h
#define __PrettyPrintTable_h
#include <stdio.h>
#include <vector>
#include <string>
#include <ostream>

#if defined( _WIN32 ) || defined( _WIN64 )
// Windows uses a different function name for this behavior.
#  define SNPRINTF_FUNC _snprintf
#else
#  define SNPRINTF_FUNC snprintf
#endif
/**
  * \class PrettyPrintTable
  * \author Kent Williams
  * Simple class to print out column-aligned tables
  */
class PrettyPrintTable
{
public:
private:
  typedef std::vector< std::string > rowType;
  typedef std::vector< rowType >     tableType;

  tableType    m_Table;
  unsigned int m_Pad;
  bool         m_rightJustify;
public:
  PrettyPrintTable():m_Pad(1),
    m_rightJustify(false)
  {}

  void setTablePad(unsigned int pad)
  {
    this->m_Pad = pad;
  }

  void leftJustify(void)
  {
    m_rightJustify = false;
  }

  void rightJustify(void)
  {
    m_rightJustify = true;
  }

  void add(const unsigned int row, const unsigned int column, const char *const s)
  {
    // Make sure the table has enough rows.
    if ( m_Table.size() <= row )
      { // Add empty rows
      m_Table.resize(row + 1);
      }
    // For each row, make sure that it now has enough columns.
    for ( unsigned int q = 0; q < m_Table.size(); q++ )
      {
      if ( m_Table[q].size() <= column )
        {
        m_Table[q].resize( column + 1, std::string("") );
        }
      }
    m_Table[row][column] = s;
  }

  void add(const unsigned int row, const unsigned int column, const std::string & s)
  {
    add( row, column, s.c_str() );
  }

  void add(const unsigned int row, const unsigned int column, const int x, const char *printf_format = 0)
  {
    const char *format(printf_format == 0 ? "%d" : printf_format);
    char        buf[4096];

    SNPRINTF_FUNC(buf, 4096, format, x);
    this->add(row, column, buf);
  }

  void add(const unsigned int row, const unsigned int column, const unsigned int x, const char *printf_format = 0)
  {
    const char *format(printf_format == 0 ? "%d" : printf_format);
    char        buf[4096];

    SNPRINTF_FUNC(buf, 4096, format, x);
    this->add(row, column, buf);
  }

  void add(const unsigned int row, const unsigned int column, const double x, const char *printf_format = 0)
  {
    const char *format(printf_format == 0 ? "%lf" : printf_format);
    char        buf[4096];

    SNPRINTF_FUNC(buf, 4096, format, x);
    this->add(row, column, buf);
  }

  void Print(std::ostream & output)
  {
    typedef std::vector< unsigned int > ColWidthsType;
    ColWidthsType colWidths(m_Table[0].size(), 0);
    // find largest columns
    for ( unsigned i = 0; i < m_Table.size(); i++ )
      {
      for ( unsigned j = 0; j < m_Table[i].size(); j++ )
        {
        if ( colWidths[j] < m_Table[i][j].size() )
          {
          colWidths[j] = m_Table[i][j].size();
          }
        }
      }
    for ( unsigned i = 0; i < m_Table.size(); i++ )
      {
      for ( unsigned j = 0; j < m_Table[i].size(); j++ )
        {
        // if right justify, output leading blanks
        if ( m_rightJustify )
          {
          int count = colWidths[j]
                      - m_Table[i][j].size();
          while ( count-- )
            {
            output << " ";
            }
          }
        unsigned int k(0);
        for ( k = 0; k < m_Table[i][j].size(); k++ )
          {
          output << m_Table[i][j][k];
          }
        unsigned int limit;
        // if right justify, just output pad
        if ( m_rightJustify )
          {
          limit = this->m_Pad;
          k = 0;
          }
        else
          {
          // print column fill + pad
          limit = colWidths[j] + this->m_Pad;
          }
        for (; k < limit; k++ )
          {
          output << " ";
          }
        }
      output << std::endl;
      }
  }
};

#endif // PrettyPrintTable_h
