#pragma once

struct PodEntry;
class Node;
class World;

std::vector<PodEntry*> FindAssetUsersOfPod(PodEntry* pod);
std::vector<Node*> FindNodeUsersOfPod(PodEntry* pod, World* world);
