#pragma once
#include "Subject.h"
class SubjectB : public Subject
{
public:
    SubjectB(void);
    ~SubjectB(void) override;

protected:

    void InitPost(void) override;

    VECTOR GetInitPos(void) override;

    ResourceManager::SRC GetModelType() const override;

    void UpdateMove(void) override;

};

