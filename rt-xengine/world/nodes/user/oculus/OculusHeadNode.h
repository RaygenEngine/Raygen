//#ifndef OCULUSHEADNODE_H
//#define OCULUSHEADNODE_H
//
//namespace World
//{
//
//	class OculusHeadNode : public Node
//	{
//		friend class OculusUserNode;
//
//		CameraNode* m_eyes[ET_COUNT];
//
//	public:
//		OculusHeadNode(Node* parent);
//		~OculusHeadNode() = default;
//
//		std::string ToString(bool verbose, uint depth) const override;
//
//		CameraNode* GetEye(EyeTarget index) const { return m_eyes[index]; }
//
//	public:
//		bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
//		bool LoadChildrenFromXML(const tinyxml2::XMLElement* xmlData) override;
//	};
//
//}
//
//#endif // OCULUSHEADNODE_H
