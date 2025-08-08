#ifndef ROWITEM_DATA_HPP
#define ROWITEM_DATA_HPP

#include <gui_generated/containers/rowItem_DataBase.hpp>

class rowItem_Data : public rowItem_DataBase
{
public:
    rowItem_Data();
    virtual ~rowItem_Data() {}

    virtual void initialize();

    void setTitle(const char *str);
    void setValue(const char *str);
    void setUnit(const char *str);
    void setCount(int count);
protected:
};

#endif // ROWITEM_DATA_HPP
