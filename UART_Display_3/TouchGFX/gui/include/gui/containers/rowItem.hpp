#ifndef ROWITEM_HPP
#define ROWITEM_HPP

#include <gui_generated/containers/rowItemBase.hpp>

class rowItem : public rowItemBase
{
public:
    rowItem();
    virtual ~rowItem() {}

    virtual void initialize();

    void setTitle(const char *str);
    void setValue(const char *str);
protected:
};

#endif // ROWITEM_HPP
