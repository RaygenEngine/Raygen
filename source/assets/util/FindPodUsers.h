#pragma once

struct PodEntry;
class World;

std::vector<PodEntry*> FindAssetUsersOfPod(PodEntry* pod);
// std::vector<Node*> FindUsersOfPod(PodEntry* pod, World* world);
