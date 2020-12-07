#include "pch.h"

#include "Intersection.hpp"
#include "StageMain.hpp"

//*******************************************************************
//Stage_IntersectionManager
//*******************************************************************
Stage_IntersectionManager::Stage_IntersectionManager(Scene* parent) : TaskBase(parent) {
	listSpace_.resize(3);
	for (size_t iSpace = 0; iSpace < listSpace_.size(); iSpace++) {
		DxRectangle<int> rect(-64, -64, SCREEN_WIDTH + 64, SCREEN_HEIGHT + 64);
		Stage_IntersectionSpace* space = new Stage_IntersectionSpace(rect);
		listSpace_[iSpace] = space;
	}
}
Stage_IntersectionManager::~Stage_IntersectionManager() {
	for (auto& iSpace : listSpace_)
		ptr_delete(iSpace);
	listSpace_.clear();
}

void Stage_IntersectionManager::Update() {
	size_t totalCheck = 0;
	size_t totalTarget = 0;

	for (auto& iSpace : listSpace_) {
		size_t countCheckThis = 0;
		auto listCheck = iSpace->CreateIntersectionCheckList(this, &countCheckThis);

		for (size_t iCheck = 0; iCheck < countCheckThis; ++iCheck) {
			Stage_IntersectionSpace::TargetCheckListPair& cTargetPair = listCheck->at(iCheck);

			shared_ptr<Stage_IntersectionTarget> targetA = cTargetPair.first;
			shared_ptr<Stage_IntersectionTarget> targetB = cTargetPair.second;
			if (targetA == nullptr || targetB == nullptr) continue;

			bool bIntersected = IsIntersected(targetA, targetB);
			if (bIntersected) {
				weak_ptr<Stage_ObjCollision> objA = targetA->GetParent();
				weak_ptr<Stage_ObjCollision> objB = targetB->GetParent();
				auto ptrA = objA.lock();
				auto ptrB = objB.lock();

				{
					if (ptrA) {
						ptrA->Intersect(targetA, targetB);
						ptrA->SetIntersected();
						if (ptrB)
							ptrA->AddIntersection(objB);
					}
					if (ptrB) {
						ptrB->Intersect(targetB, targetA);
						ptrB->SetIntersected();
						if (ptrA)
							ptrB->AddIntersection(objA);
					}
				}
			}
		}

		totalCheck += countCheckThis;
		iSpace->ClearTarget();

		std::fill(listCheck->begin(), listCheck->begin() + countCheckThis, Stage_IntersectionSpace::TargetCheckListPair());
	}
}

void Stage_IntersectionManager::AddTarget(Stage_IntersectionTarget::TypeTarget type, shared_ptr<Stage_IntersectionTarget> target) {
	if (target == nullptr) return;
	switch (type) {
	case Stage_IntersectionTarget::TypeTarget::Player:
		listSpace_[SPACE_PLAYER_ENEMY]->RegistTargetA(target);
		break;
	case Stage_IntersectionTarget::TypeTarget::PlayerShot:
	case Stage_IntersectionTarget::TypeTarget::PlayerSpell:
		listSpace_[SPACE_PLAYERSHOT_ENEMY]->RegistTargetA(target);
		break;
	case Stage_IntersectionTarget::TypeTarget::EnemyToPlayer:
		listSpace_[SPACE_PLAYER_ENEMY]->RegistTargetB(target);
		break;
	case Stage_IntersectionTarget::TypeTarget::EnemyToPlayerShot:
		listSpace_[SPACE_PLAYERSHOT_ENEMY]->RegistTargetB(target);
		break;
	case Stage_IntersectionTarget::TypeTarget::EnemyShot:
		listSpace_[SPACE_PLAYER_ENEMY]->RegistTargetB(target);
		break;
	}
}
bool Stage_IntersectionManager::IsIntersected(shared_ptr<Stage_IntersectionTarget>& target1, 
	shared_ptr<Stage_IntersectionTarget>& target2) 
{
	if (target1 == nullptr || target2 == nullptr) return false;
	Stage_IntersectionTarget* p1 = target1.get();
	Stage_IntersectionTarget* p2 = target2.get();

	{
		Stage_IntersectionTarget_Circle* c1 = dynamic_cast<Stage_IntersectionTarget_Circle*>(p1);
		Stage_IntersectionTarget_Circle* c2 = dynamic_cast<Stage_IntersectionTarget_Circle*>(p2);
		return DxCollider::Test(c1->GetCircle(), c2->GetCircle());
	}
}

//*******************************************************************
//Stage_IntersectionSpace
//*******************************************************************
Stage_IntersectionSpace::Stage_IntersectionSpace(const DxRectangle<int>& space) {
	spaceRect_ = space;

	pooledCheckList_.resize(64U);
}

bool Stage_IntersectionSpace::RegistTarget(ListTarget* pVec, shared_ptr<Stage_IntersectionTarget>& target) {
	if (!DxCollider::Test(spaceRect_, target->GetIntersectionSpaceRect()))
		return false;
	pVec->push_back(target);
	return true;
}
void Stage_IntersectionSpace::ClearTarget() {
	pairTargetList_.first.clear();
	pairTargetList_.second.clear();
}

std::vector<Stage_IntersectionSpace::TargetCheckListPair>*
Stage_IntersectionSpace::CreateIntersectionCheckList(Stage_IntersectionManager* manager, size_t* pTotal) {
	ListTarget* pListTargetA = &pairTargetList_.first;
	ListTarget* pListTargetB = &pairTargetList_.second;

	std::atomic_uint count = 0;

	if (pListTargetA->size() > 0 && pListTargetB->size() > 0) {
		static std::mutex mtx;

		auto CheckSpaceRect = [&](shared_ptr<Stage_IntersectionTarget>& targetA, shared_ptr<Stage_IntersectionTarget>& targetB) {
			const DxRectangle<int>& boundA = targetA->GetIntersectionSpaceRect();
			const DxRectangle<int>& boundB = targetB->GetIntersectionSpaceRect();

			if (DxCollider::Test(boundA, boundB)) {
				if (count >= pooledCheckList_.size()) {
					mtx.lock();
					pooledCheckList_.resize(pooledCheckList_.size() * 2);
					mtx.unlock();
				}
				pooledCheckList_[count] = std::make_pair(targetA, targetB);
				++count;
			}
		};

		//Attempt to most efficiently utilize multithreading
		if (pListTargetA->size() > pListTargetB->size()) {
			ParallelTask(pListTargetA->size(), [&](size_t iA) {
				shared_ptr<Stage_IntersectionTarget>& pTargetA = pListTargetA->at(iA);
				for (auto itrB = pListTargetB->begin(); itrB != pListTargetB->end(); ++itrB) {
					shared_ptr<Stage_IntersectionTarget>& pTargetB = *itrB;
					CheckSpaceRect(pTargetA, pTargetB);
				}
			});
		}
		else {
			ParallelTask(pListTargetB->size(), [&](size_t iB) {
				shared_ptr<Stage_IntersectionTarget>& pTargetB = pListTargetB->at(iB);
				for (auto itrA = pListTargetA->begin(); itrA != pListTargetA->end(); ++itrA) {
					shared_ptr<Stage_IntersectionTarget>& pTargetA = *itrA;
					CheckSpaceRect(pTargetA, pTargetB);
				}
			});
		}
	}

	*pTotal = count;
	return &pooledCheckList_;
}

//*******************************************************************
//Stage_ObjCollision
//*******************************************************************
void Stage_ObjCollision::AddRelativeTarget(shared_ptr<Stage_IntersectionTarget> target) {
	RelativeTarget newTarget;
	{
		DxShapeBase* pOrgShape = new DxCircle<float>();
		if (Stage_IntersectionTarget_Circle* pTarget = dynamic_cast<Stage_IntersectionTarget_Circle*>(target.get()))
			pOrgShape = new DxCircle<float>(pTarget->GetCircle());
		newTarget.orgShape = pOrgShape;		//Original
	}
	newTarget.orgIntersectionRect = target->GetIntersectionSpaceRect();
	newTarget.relTarget = target;			//Relative
	listRelativeTarget_.push_back(newTarget);
}
void Stage_ObjCollision::ClearRelativeTarget() {
	for (auto& iTargetPair : listRelativeTarget_)
		delete iTargetPair.orgShape;
	listRelativeTarget_.clear();
}

void Stage_ObjCollision::UpdateRelativeTarget(float posX, float posY) {
	for (auto& iTargetList : listRelativeTarget_) {
		const DxShapeBase* pShapeOrg = iTargetList.orgShape;
		const DxRectangle<int>& pRectOrg = iTargetList.orgIntersectionRect;
		Stage_IntersectionTarget* targetRel = iTargetList.relTarget.get();

		{
			const DxCircle<float>* pCircleOrg = dynamic_cast<const DxCircle<float>*>(pShapeOrg);
			DxCircle<float>& shapeRel = dynamic_cast<Stage_IntersectionTarget_Circle*>(targetRel)->GetCircle();

			shapeRel.x = pCircleOrg->x + posX;
			shapeRel.y = pCircleOrg->y + posY;
		}

		targetRel->SetIntersectionSpace(pRectOrg + DxRectangle<int>::SetFromSize(posX, posY));
	}
}
void Stage_ObjCollision::RegistRelativeTarget(Stage_IntersectionManager* manager) {
	for (auto& iTargetList : listRelativeTarget_) {
		if (iTargetList.orgShape)
			manager->AddTarget(iTargetList.relTarget->GetTargetType(), iTargetList.relTarget);
	}
}

//*******************************************************************
//Stage_IntersectionTarget
//*******************************************************************
Stage_IntersectionTarget::Stage_IntersectionTarget() {
	type_ = TypeTarget::None;
	intersectionSpace_ = DxRectangle<int>(0, 0, 0, 0);
}
void Stage_IntersectionTarget::ClearObjectIntersectedIdList() {
	if (auto ptr = parent_.lock())
		ptr->ClearIntersectedList();
}

//*******************************************************************
//Stage_IntersectionTarget_Circle
//*******************************************************************
Stage_IntersectionTarget_Circle::Stage_IntersectionTarget_Circle() {
	circle_ = DxCircle<float>(0, 0, 0);
}
void Stage_IntersectionTarget_Circle::SetIntersectionSpace() {
	constexpr int MARGIN = 4;
	int r = circle_.r + 0.0001f + MARGIN;
	intersectionSpace_ = DxRectangle<int>(circle_.x - r, circle_.y - r, 
		circle_.x + r, circle_.y + r);
}

//*******************************************************************
//
//*******************************************************************