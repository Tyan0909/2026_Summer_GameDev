#pragma once
#include "Subject.h"
class SubjectC : public Subject
{
public:
    SubjectC(void);
    ~SubjectC(void) override;

protected:

    void InitPost(void) override;
	void InitLoad(void) override;
	void InitTransform(void) override;

    VECTOR GetInitPos(void) override;

    ResourceManager::SRC GetModelType() const override;

    void UpdateMove(void) override;

};

