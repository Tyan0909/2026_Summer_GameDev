#pragma once
#include "Subject.h"
class SubjectB : public Subject
{
protected:

	void InitPost(void) override;

	VECTOR GetInitPos(void) override;

	 ResourceManager::SRC GetModelType() const override;

	void UpdateMove(void) override;

};

