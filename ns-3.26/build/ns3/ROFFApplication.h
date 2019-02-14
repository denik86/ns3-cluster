/*
 * ROFFApplication.h
 *
 *  Created on: Jan 28, 2019
 *      Author: jordan
 */

#ifndef ROFFAPPLICATION_H
#define ROFFAPPLICATION_H

#include "ROFFHeader.h"
#include "ROFFNode.h"
#include "ns3/application.h"
#include "ns3/network-module.h"

namespace ns3 {

/**
 * \ingroup network
 * \brief A special application that implements ROFF protocol.
 */
class ROFFApplication: public Application {

public:
	ROFFApplication ();
	virtual ~ROFFApplication();

	static TypeId GetTypeId();

	/**
	* \brief Set up some application parameters
	* \param protocol fb or static protocol
	* \param broadcastPhaseStart time after which broadcast phase will start (seconds)
	* \param actualRange actual transmission range (meters)
	* \param aoi radius of area of interest (meters)
	* \param aoi_error distance +/- with respect to the radius (meters)
	* \param flooding enable or disable flooding
	* \param cwMin minumum size of the contention window (slots)
	* \param cwMin maximum size of the contention window (slots)
	* \return none
	*/
	void Install();

	/**
	* \brief Add a new node to the applicatin and set up protocol parameters
	* \param node node to add
	* \param source source socket of the node
	* \param sink sink socket of the node
	* \param onstats if this node is a vehicle
	* \return none
	*/
	void AddNode(Ptr<Node> node, Ptr<Socket> source, Ptr<Socket> sink);

	/**
	* \brief Print value of some useful field
	* \param dataStream output data
	* \return none
	*/
	void PrintStats(std::stringstream& dataStream);

private:
	/**
	* \brief Application specific startup code
	*
	* The StartApplication method is called at the start time specified by Start
	* This method should be overridden by all or most application
	* subclasses.
	*/
	virtual void StartApplication(void);

	/**
	* \brief Application specific shutdown code
	*
	* The StopApplication method is called at the stop time specified by Stop
	* This method should be overridden by all or most application
	* subclasses.
	*/
	virtual void StopApplication (void);

	/**
	* \brief Start the estimation phase
	* \param count count
	* \return none
	*/
	void GenerateHelloTraffic (uint32_t count);

	/**
	* \brief Start the broadcast phase
	* \return none
	*/
	void StartBroadcastPhase (void);

	/**
	 * \brief Send a Hello message to all nodes in its range
	 * \return none
	 */
	void GenerateHelloMessage (Ptr<ROFFNode> fbNode);

	/**
	* \brief Send a Alert message
	* \return none
	*/
	void GenerateAlertMessage (Ptr<ROFFNode> fbNode);

	/**
	* \brief Process a received packet
	* \param socket the receiving socket
	* \return none
	*/
	void ReceivePacket (Ptr<Socket> socket);

	/**
	* \brief Handle a Hello message
	* \param fbNode node that received the message
	* \param fbHeader header received in the message
	* \return none
	*/
	void HandleHelloMessage (Ptr<ROFFNode> fbNode, ROFFNode fbHeader);

	/**
	* \brief Handle an Alert message
	* \param fbNode node that received the message
	* \param fbHeader header received in the message
	* \param distance distance between the sender of the message and the node (meters)
	* \return none
	*/
	void HandleAlertMessage (Ptr<ROFFNode> fbNode, ROFFNode fbHeader, uint32_t distance);

	/**
	* \brief Calculate min diff between nodes
	* \return mindiff
	*/
	double CalculateMinDiff();

	/**
	* \brief Wait a specific amount of time
	* \param fbNode node that received the message
	* \param fbHeader header received in the message
	* \param waitingTime contention window value
	* \return none
	*/
	void WaitAgain (Ptr<ROFFNode> fbNode,  ROFFNode fbHeader, uint32_t waitingTime);

	/**
	* \brief Forward an Alert message
	* \param fbNode node that received the message
	* \param fbHeader header received in the message
	* \param waitingTime contention window value
	* \return none
	*/
	void ForwardAlertMessage (Ptr<ROFFNode> fbNode, ROFFNode oldFBHeader, uint32_t waitingTime);

	/**
	* \brief Stop a node
	* \param fbNode node to be stopped
	* \return none
	*/
	void StopNode (Ptr<ROFFNode> fbNode);

	/**
	* \brief Retrieve a fbNode from a ns3::Node
	* \param node original node
	* \return a fbNode
	*/
	Ptr<ROFFNode> GetFBNode (Ptr<Node> node);

	/**
	* \brief Retrieve a fbNode from a ns3::Node
	* \param id original node id
	* \return a fbNode
	*/
	Ptr<ROFFNode> GetFBNode (uint32_t id);

	/**
	* \brief Compute contention window
	* \param maxRange estimated range (meters)
	* \param distance distance between nodes (meters)
	* \return the value of the contention window
	*/
	uint32_t ComputeContentionWindow (uint32_t maxRange, uint32_t distance);

private:
	uint32_t 						m_nnodes;
	std::vector<Ptr<ROFFNode>>		m_nodes;
	uint32_t						m_startingNode;

};

}



#endif /* ROFFAPPLICATION_H_*/