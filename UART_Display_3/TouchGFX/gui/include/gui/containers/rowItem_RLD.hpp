#ifndef ROWITEM_RLD_HPP
#define ROWITEM_RLD_HPP

#include <gui_generated/containers/rowItem_RLDBase.hpp>

class rowItem_RLD : public rowItem_RLDBase
{
public:
    rowItem_RLD();
    virtual ~rowItem_RLD() {}

    virtual void initialize();

    void setTitle(const char *str);
    void setValue(const char *str);

protected:
};

#endif // ROWITEM_RLD_HPP
