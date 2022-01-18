#include "cell_move_router/Mover/Mover.hpp"
#include "cell_move_router/Router/GraphApproxRouter.hpp"
#include "Third/Flute3-Cpp-Wrapper/Flute3Wrapper.hpp"
#include <unordered_map>
#include <algorithm>
#include <climits>
#include <iostream>

namespace cell_move_router {
namespace Mover {

void Mover::initalFreqMovedCell() {
  for (auto &Cell : InputPtr->getCellInsts()) {
    if (Cell.isMovable()) {
      if (GridManager.getCellVoltageArea(&Cell).size())
        continue; // TODO: handle Cell in VoltageArea
      FreqMovedCell.emplace(&Cell, 0);
    }
  }
}
bool Mover::add_and_route(const Input::Processed::CellInst *CellPtr,
                          const int Row, const int Col) {
  GridManager.addCell(CellPtr, Row, Col);
  if (GridManager.isOverflow()) {
    GridManager.removeCell(CellPtr);
    return false;
  }
  Router::GraphApproxRouter GraphApproxRouter(&GridManager);
  std::vector<std::pair<
      const Input::Processed::Net *,
      std::pair<std::vector<cell_move_router::Input::Processed::Route>,
                long long>>>
      OriginRoutes;
  bool Accept = true;
  for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
    auto &OriginRoute = GridManager.getNetRoutes()[NetPtr];
    auto Pair = GraphApproxRouter.singleNetRoute(NetPtr, OriginRoute.first);
    OriginRoutes.emplace_back(NetPtr, std::move(OriginRoute));
    if (Pair.second == false) {
      Accept = false;
      break;
    }
    auto Cost = GridManager.getRouteCost(NetPtr, Pair.first);
    Input::Processed::Route::reduceRouteSegments(Pair.first);
    OriginRoute = {std::move(Pair.first), Cost};
    bool Overflow = GridManager.isOverflow();
    GridManager.addNet(NetPtr);
    assert(!Overflow);
  }
  if (Accept) {
    return true;
  }
  GridManager.getNetRoutes()[OriginRoutes.back().first] =
      std::move(OriginRoutes.back().second);
  OriginRoutes.pop_back();
  while (OriginRoutes.size()) {
    GridManager.removeNet(OriginRoutes.back().first);
    GridManager.getNetRoutes()[OriginRoutes.back().first] =
        std::move(OriginRoutes.back().second);
    OriginRoutes.pop_back();
  }
  GridManager.removeCell(CellPtr);
  return false;
}
void Mover::move(RegionCalculator::RegionCalculator &RC) {
  std::vector<std::pair<long long, const Input::Processed::CellInst *>>
      CellNetLength;
  for (auto &P : FreqMovedCell) {
    auto CellPtr = P.first;
    long long NetLength = 0;
    for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
      NetLength += GridManager.getNetRoutes()[NetPtr].second;
    }
    CellNetLength.emplace_back(NetLength, CellPtr);
  }
  std::sort(
      CellNetLength.begin(), CellNetLength.end(),
      std::greater<std::pair<long long, const Input::Processed::CellInst *>>());
  unsigned MoveCnt = 0;
  for (auto &P : CellNetLength) {
    auto CellPtr = P.second;
    int RowBeginIdx = 0, RowEndIdx = 0, ColBeginIdx = 0, ColEndIdx = 0;
    std::tie(RowBeginIdx, RowEndIdx, ColBeginIdx, ColEndIdx) =
        RC.getRegion(CellPtr);
    std::vector<std::pair<int, int>> CandidatePos;
    for (int R = RowBeginIdx; R <= RowEndIdx; ++R) {
      for (int C = ColBeginIdx; C <= ColEndIdx; ++C) {
        CandidatePos.emplace_back(R, C);
      }
    }
    int id = 0;
    std::vector<std::vector<int>> row;
    std::vector<std::vector<int>> col;
    std::vector<std::pair<int, int>> FLUTE_WL;
    auto Wrapper = Flute::FluteWrapper::getInstance();
    auto OldCoord = GridManager.getCellCoordinate(CellPtr);
    
    for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
      std::vector<int> temp_x;
      std::vector<int> temp_y;
      for (auto p: NetPtr->getPins()) {
        if (p.getInst()==CellPtr) continue;
        temp_x.emplace_back(p.getInst()->getGGridRowIdx());
        temp_y.emplace_back(p.getInst()->getGGridColIdx());
      }
      row.emplace_back(temp_x);
      col.emplace_back(temp_y);
      GridManager.removeNet(NetPtr);
    }
    GridManager.removeCell(CellPtr);

    bool Success = false;
    for (auto P : CandidatePos) {
      int WL = 0;
      for (int i=0;i<int(row.size());i++) {
        row[i].emplace_back(P.first);
        col[i].emplace_back(P.second);
        auto FluteTree = Wrapper->runFlute(row[i], col[i]);
        WL += FluteTree.getLength();
        row[i].pop_back();
        col[i].pop_back();
      }
      FLUTE_WL.emplace_back(WL,id);
      id++;
    }
    std::sort(FLUTE_WL.begin(),FLUTE_WL.end());
    for (auto F : FLUTE_WL) {
      if (add_and_route(CellPtr, CandidatePos[F.second].first, CandidatePos[F.second].second)) {
        Success = true;
        break;
      }
    }
    if (Success) {
      ++MoveCnt;
      // std::cerr<<"done"<<MoveCnt<<"\n";
    }
    else {
      GridManager.addCell(CellPtr, OldCoord.first, OldCoord.second);
      for (auto NetPtr : InputPtr->getRelativeNetsMap().at(CellPtr)) {
        GridManager.addNet(NetPtr);
      }
    }
    if (MoveCnt == InputPtr->getMaxCellMove())
      break;
  }
}

} // namespace Mover
} // namespace cell_move_router