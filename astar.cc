#include <queue>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <set>
#include <bitset>

constexpr uint16_t maxNodes = 100;
constexpr uint64_t maxFuel = 50;

uint64_t dist(uint16_t aNode1, uint16_t aNode2)
{
  return (uint64_t) std::abs((int) aNode2 - (int) aNode1);
}

uint64_t chargeTime(uint64_t aFuelLeft)
{
  if (aFuelLeft >= maxFuel)
    return 0;

  return maxFuel - aFuelLeft;
}

uint64_t travelTime(uint16_t aNode1, uint16_t aNode2)
{
  return dist(aNode1, aNode2);
}

uint64_t fuelConsumption(uint16_t aNode1, uint16_t aNode2)
{
  return dist(aNode1, aNode2);
}

uint64_t estimateTravelTime(uint16_t aNode1, uint16_t aNode2)
{
  return dist(aNode1, aNode2);
}


struct TLabel
{
  uint16_t iArrivalNode;
  uint64_t iTravelTimeFromOrigin; // Travel + recharging time when first arriving here
  uint64_t iEstimatedTravelTimeToDestination; // Recharging here + travelleing on
  uint64_t iFuelLeftUponArrival;
  time_t   iArrivalTime;
};

struct TLabelCompare
{
  bool operator()(const TLabel& aLeft, const TLabel& aRight) const
  {
    return (aLeft.iTravelTimeFromOrigin + aLeft.iEstimatedTravelTimeToDestination) > (aRight.iTravelTimeFromOrigin + aRight.iEstimatedTravelTimeToDestination);
  }
};

class CCandidateFilter
{
public:
  class  CCandidateFilterIterator
  {
  public:
    CCandidateFilterIterator(uint16_t aCenterNode,  uint16_t aDestinationNode, uint64_t aMaxDistance)
      : iCurrentNode(0), iStartNode(aCenterNode), iDestinationNode(aDestinationNode), iMaxDistance(aMaxDistance)
    {
      if (aCenterNode == maxNodes)
	iCurrentNode = maxNodes;
      else if (aCenterNode == 0 || dist(0, aCenterNode) > iMaxDistance)
        this->operator++();
    }

    const CCandidateFilterIterator& operator++()
    {
      if (iCurrentNode >= maxNodes)
        return *this;

      ++iCurrentNode;
      
      while(iCurrentNode < maxNodes && (dist(iStartNode, iCurrentNode) > iMaxDistance || iCurrentNode == iStartNode))
      {
        iCurrentNode++;
      }

      return *this;
    }

    CCandidateFilterIterator operator++(int)
    {
      CCandidateFilterIterator result = *this;
      if (iCurrentNode >= maxNodes)
        return result;

      ++iCurrentNode;     
      while(iCurrentNode < maxNodes && (dist(iStartNode, iCurrentNode) > iMaxDistance || iCurrentNode == iStartNode))
      {
        iCurrentNode++;
      }

      return result;
    } 

    uint16_t operator*() const
    {
      return iCurrentNode;
    }

    bool operator==(const CCandidateFilterIterator& aOther) const
    {
      return iCurrentNode == aOther.iCurrentNode;
    }

    bool operator!=(const CCandidateFilterIterator& aOther) const
    {
      return iCurrentNode != aOther.iCurrentNode;
    }

  private:
    uint16_t iCurrentNode;
    uint16_t iStartNode;
    uint16_t iDestinationNode;
    uint64_t iMaxDistance;
  };

  CCandidateFilter(uint16_t aCenterNode, uint16_t aDestinationNode, uint64_t aMaxDistance)
    : iCurrentNode(aCenterNode), iDestinationNode(aDestinationNode), iMaxDistance(aMaxDistance)
  {
    
  }

  CCandidateFilterIterator begin()
  {
    return CCandidateFilterIterator(iCurrentNode, iDestinationNode, iMaxDistance);
  }

  CCandidateFilterIterator end()
  {
    return CCandidateFilterIterator(maxNodes, iDestinationNode, iMaxDistance);
  }

private:
  uint16_t iCurrentNode;
  uint16_t iDestinationNode;
  uint64_t iMaxDistance;
};

int main(int argc, char** argv)
{ 
  time_t rawtime;
  time ( &rawtime );
  uint64_t popCount = 0;
  uint16_t destinationNode = 31;
  std::set<uint16_t> closedSet;
  const TLabel startLabel = {0, 0, estimateTravelTime(0, destinationNode), maxFuel, rawtime};
  std::priority_queue<TLabel, std::vector<TLabel>, TLabelCompare> q;
  q.push(startLabel);

  while(!q.empty())
  {
    TLabel bestCandidate = q.top();
    std::cout << "Examining " << bestCandidate.iArrivalNode << " (" << bestCandidate.iTravelTimeFromOrigin << ", " << bestCandidate.iEstimatedTravelTimeToDestination << ")" << std::endl;
    q.pop();
    popCount++;
    if (bestCandidate.iArrivalNode == destinationNode)
    {
      char departureBuf[80];
      char arrivalBuf[80];
      struct tm departureTime = *localtime(&rawtime);
      time_t arrivalEpoch = bestCandidate.iArrivalTime;
      struct tm arrivalTime = *localtime(&arrivalEpoch);
      strftime(departureBuf, 79, "%FT%T%z", &departureTime);
      strftime(arrivalBuf, 79, "%FT%T%z", &arrivalTime); 
      std::cout << "Destination " << destinationNode <<  " reached! Departed at " << departureBuf << ", arriving at " << arrivalBuf << " (travel time: " << bestCandidate.iTravelTimeFromOrigin << " seconds)" << std::endl;
      std::cout << "Pop count is " << popCount << std::endl;
      break;
    } 

    if (closedSet.count(bestCandidate.iArrivalNode) > 0)
    {
      continue;
    } 

    for(auto candidateNextStop : CCandidateFilter(bestCandidate.iArrivalNode, destinationNode, 5))
    {
      TLabel successorLabel;
      successorLabel.iArrivalNode = candidateNextStop;
      time_t timeToNextNode = chargeTime(bestCandidate.iFuelLeftUponArrival) + travelTime(bestCandidate.iArrivalNode, candidateNextStop);
      successorLabel.iArrivalTime = bestCandidate.iArrivalTime + timeToNextNode;
      successorLabel.iFuelLeftUponArrival = maxFuel - fuelConsumption(bestCandidate.iArrivalNode, candidateNextStop);
      successorLabel.iTravelTimeFromOrigin = bestCandidate.iTravelTimeFromOrigin + timeToNextNode;
      successorLabel.iEstimatedTravelTimeToDestination = estimateTravelTime(candidateNextStop, destinationNode);
      q.push(successorLabel);
    }
  
    closedSet.insert(bestCandidate.iArrivalNode);
  }
  
  return 0;
}
