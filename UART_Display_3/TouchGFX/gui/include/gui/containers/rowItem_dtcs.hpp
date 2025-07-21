#ifndef ROWITEM_DTCS_HPP
#define ROWITEM_DTCS_HPP

#include <gui_generated/containers/rowItem_dtcsBase.hpp>

class rowItem_dtcs : public rowItem_dtcsBase
{
public:
    rowItem_dtcs();
    virtual ~rowItem_dtcs() {}

    virtual void initialize();

    void dtcs_Desc_set(const char *str);

    void dtcs_set(const char *str);

protected:
};

#endif // ROWITEM_DTCS_HPP
