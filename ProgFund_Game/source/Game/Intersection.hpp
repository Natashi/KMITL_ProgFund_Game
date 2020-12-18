#pragma once
#include "../../pch.h"

#include "System.hpp"

enum class IntersectPolarity : byte {
	None,
	White,
	Black,
};

class Stage_IntersectionManager;
class Stage_ObjCollision;
class Stage_IntersectionTarget {
	friend class Stage_IntersectionManager;
public:
	enum class TypeTarget : byte {
		None,
		Player,
		PlayerShot,
		PlayerSpell,
		EnemyToPlayer,
		EnemyToPlayerShot,
		EnemyShot,
	};
protected:
	TypeTarget type_;
	weak_ptr<Stage_ObjCollision> parent_;

	DxRectangle<int> intersectionSpace_;
public:
	Stage_IntersectionTarget();

	const DxRectangle<int>& GetIntersectionSpaceRect() const { return intersectionSpace_; }
	void SetIntersectionSpace(const DxRectangle<int>& rect) { intersectionSpace_ = rect; }
	virtual void SetIntersectionSpace() = 0;

	TypeTarget GetTargetType() const { return type_; }
	void SetTargetType(TypeTarget type) { type_ = type; }

	weak_ptr<Stage_ObjCollision> GetParent() { return parent_; }
	void SetParent(weak_ptr<Stage_ObjCollision> obj) {
		if (!obj.expired()) parent_ = obj;
	}

	void ClearObjectIntersectedIdList();
};
class Stage_IntersectionTarget_Circle : public Stage_IntersectionTarget {
	friend class Stage_IntersectionManager;
protected:
	DxCircle<float> circle_;
public:
	Stage_IntersectionTarget_Circle();

	virtual void SetIntersectionSpace();

	DxCircle<float>& GetCircle() { return circle_; }
	void SetCircle(const DxCircle<float>& circle) {
		circle_ = circle; SetIntersectionSpace();
	}
};

class Stage_MainScene;

class Stage_IntersectionManager;
class Stage_IntersectionSpace;

class Stage_ObjCollision;

class Stage_IntersectionManager : public TaskBase {
	friend class Stage_MainScene;
public:
	typedef enum : byte {
		SPACE_PLAYER_ENEMY,
		SPACE_PLAYERSHOT_ENEMY,
	} TypeSpace;
protected:
	std::vector<Stage_IntersectionSpace*> listSpace_;
public:
	Stage_IntersectionManager(Scene* parent);
	~Stage_IntersectionManager();

	virtual void Update();

	void AddTarget(Stage_IntersectionTarget::TypeTarget type, shared_ptr<Stage_IntersectionTarget> target);
	void AddTarget(shared_ptr<Stage_IntersectionTarget> target) {
		AddTarget(target->GetTargetType(), target);
	}

	static bool IsIntersected(Stage_IntersectionTarget* target1, Stage_IntersectionTarget* target2);
};

class Stage_IntersectionSpace {
	friend class Stage_IntersectionManager;
public:
	enum : byte {
		TYPE_A = 0,
		TYPE_B = 1,
	};
	typedef std::vector<shared_ptr<Stage_IntersectionTarget>> ListTarget;
	typedef std::pair<Stage_IntersectionTarget*, Stage_IntersectionTarget*> TargetCheckListPair;
protected:
	std::pair<ListTarget, ListTarget> pairTargetList_;
	DxRectangle<int> spaceRect_;

	std::vector<TargetCheckListPair> pooledCheckList_;
public:
	Stage_IntersectionSpace(const DxRectangle<int>& space);

	bool RegistTarget(ListTarget* pVec, shared_ptr<Stage_IntersectionTarget>& target);
	bool RegistTargetA(shared_ptr<Stage_IntersectionTarget>& target) { return RegistTarget(&pairTargetList_.first, target); }
	bool RegistTargetB(shared_ptr<Stage_IntersectionTarget>& target) { return RegistTarget(&pairTargetList_.second, target); }
	void ClearTarget();

	std::vector<TargetCheckListPair>* CreateIntersectionCheckList(Stage_IntersectionManager* manager, size_t* pTotal);
};

class Stage_ObjCollision {
	friend class Stage_IntersectionManager;
public:
	struct RelativeTarget {
		DxShapeBase* orgShape;
		DxRectangle<int> orgIntersectionRect;
		shared_ptr<Stage_IntersectionTarget> relTarget;
	};
protected:
	bool bIntersected_;
	size_t intersectedCount_;
	std::vector<weak_ptr<Stage_ObjCollision>> listIntersectedObj_;

	std::vector<RelativeTarget> listRelativeTarget_;
public:
	Stage_ObjCollision() { ClearIntersected(); }

	virtual void Intersect(Stage_IntersectionTarget* ownTarget, Stage_IntersectionTarget* otherTarget) = 0;

	void ClearIntersected() { bIntersected_ = false; intersectedCount_ = 0; }
	bool IsIntersected() { return bIntersected_; }
	void SetIntersected() { bIntersected_ = true; ++intersectedCount_; }

	size_t GetIntersectedCount() { return intersectedCount_; }
	void ClearIntersectedList() { listIntersectedObj_.clear(); }
	void AddIntersection(weak_ptr<Stage_ObjCollision> obj) { listIntersectedObj_.push_back(obj); }
	std::vector<weak_ptr<Stage_ObjCollision>>& GetIntersectedList() { return listIntersectedObj_; }

	void AddRelativeTarget(shared_ptr<Stage_IntersectionTarget> target);
	void ClearRelativeTarget();
	shared_ptr<Stage_IntersectionTarget> GetIntersectionRelativeTarget(size_t index) { return listRelativeTarget_[index].relTarget; }

	void UpdateRelativeTarget(float posX, float posY);
	void RegistRelativeTarget(Stage_IntersectionManager* manager);
	size_t GetRelativeTargetCount() { return listRelativeTarget_.size(); }
};