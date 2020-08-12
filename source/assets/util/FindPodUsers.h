#pragma once

struct PodEntry;
class Node;
class World;

std::vector<PodEntry*> FindAssetUsersOfPod(PodEntry* pod);
// std::vector<Node*> FindUsersOfPod(PodEntry* pod, World* world);
