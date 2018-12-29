/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Marco Romanelli <marco.romanelli.1@studenti.unipd.it>
 *
 */

/* -----------------------------------------------------------------------------
*			HEADERS
* ------------------------------------------------------------------------------
*/

#include <bits/stdint-uintn.h>
#include <bits/types/struct_timeval.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/time.h>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include <thread>

#include "ns3/core-module.h"
#include "ns3/node-list.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/topology.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
//#include "ns3/netanim-module.h"
#include "ns3/FBApplication.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("fb-vanet-urban");

/* -----------------------------------------------------------------------------
*			CLASS AND METHODS PROTOTIPES
* ------------------------------------------------------------------------------
*/

class CSVManager
{
public:
	/**
	 * \brief Constructor
	 * \return none
	 */
	CSVManager ();

	/**
	 * \brief Destructor
	 * \return none
	 */
	~CSVManager ();

	/**
	 * \brief Set the filename of the csv file
	 * \param filename path of the file
	 * \return none
	 */
	void Setup (std::string filename);

	/**
	 * \brief Write the header of the csv
	 * \param header header of the csv
	 * \return none
	 */
	void WriteHeader(std::string header);

	/**
	 * \brief Create a new filename adding a timestamp to a provided base
	 * \param base first part of the new filename
	 * \return none
	 */
	void EnableAlternativeFilename(boost::filesystem::path path);

	/**
	 * \brief Add a value (cell) in the current row
	 * \param value int value to be written
	 * \return none
	 */
	void AddValue(int value);

	/**
	 * \brief Add a value (cell) in the current row
	 * \param value double value to be written
	 * \return none
	 */
	void AddValue(double value);

	/**
	 * \brief Add a value (cell) in the current row
	 * \param value string value to be written
	 * \return none
	 */
	void AddValue(std::string value);

	/**
	 * \brief Add a value (cell) in the current row
	 * \param value stream value to be written
	 * \return none
	 */
	void AddValue(std::stringstream value);

	/**
	 * \brief Add multiple values in the current row
	 * \param value stream to be written
	 * \return none
	 */
	void AddMultipleValues(std::stringstream& value);

	/**
	 * \brief Write the current row and initilize a new one
	 * \return none
	 */
	void CloseRow(void);

private:
	boost::filesystem::path					m_csvFilePath;
	std::stringstream						m_currentRow;
//	std::mutex 									m_mutex;
	unsigned int 							runId;
};

CSVManager::CSVManager ()
:	m_csvFilePath (""), runId(0)
{
	NS_LOG_FUNCTION (this);
}

CSVManager::~CSVManager ()
{
	NS_LOG_FUNCTION (this);
}

void
CSVManager::Setup (std::string filename)
{
	NS_LOG_FUNCTION (this);

	m_csvFilePath = filename;
}

void
CSVManager::WriteHeader (std::string header)
{
	NS_LOG_FUNCTION (this);

	boost::filesystem::path parentPath = m_csvFilePath.parent_path();
	boost::filesystem::create_directories(parentPath);
	std::ofstream out (m_csvFilePath.string());
	out << header.c_str() << std::endl;
	out.close ();
}

void
CSVManager::EnableAlternativeFilename(boost::filesystem::path path)
{
	NS_LOG_FUNCTION (this);
	std::string separators = "/_,.";
	std::string extension = ".csv";

	// Get unix time
	struct timeval tp;
	gettimeofday(&tp, NULL);
	long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;

	// Create the new filename
//	new_filename.append(base);
//	new_filename.append(separators, 1, 1);	// only '_'
//	new_filename.append(std::to_string(ms));
	std::string finalPartOfPath = "-" + to_string(ms) + extension;
	m_csvFilePath = path += finalPartOfPath;
}


void
CSVManager::AddValue(std::stringstream value)
{
	NS_LOG_FUNCTION (this);

	m_currentRow << value.str() << ",";
}

void
CSVManager::AddValue(int value)
{
	NS_LOG_FUNCTION (this);

	m_currentRow << value << ",";
}

void
CSVManager::AddValue(double value)
{
	NS_LOG_FUNCTION (this);

	m_currentRow << value << ",";
}

void
CSVManager::AddValue(std::string value)
{
	NS_LOG_FUNCTION (this);

	m_currentRow << value << ",";
}

void
CSVManager::AddMultipleValues(std::stringstream& value)
{
	NS_LOG_FUNCTION (this);

	m_currentRow << value.str();
}

void
CSVManager::CloseRow (void)
{
	NS_LOG_FUNCTION (this);
//	m_mutex.lock();
	// write current row to file in an atomic way
	std::ofstream out (m_csvFilePath.c_str (), std::ios::app);
	out << runId << ","<< m_currentRow.rdbuf() << std::endl;
	runId++;
	out.close ();
//	m_mutex.unlock();
	// Delete (old) row
	m_currentRow.str("");
}

CSVManager			g_csvData; // CSV file manager

/**
 * \ingroup obstacle
 * \brief The VanetRoutingExperiment class implements an application that
 * allows this VANET experiment to be simulated
 */
class FBVanetExperiment
{
public:
	/**
	 * \brief Constructor
	 * \return none
	 */
	FBVanetExperiment ();

	/**
	 * \brief Destructor
	 * \return none
	 */
	virtual ~FBVanetExperiment ();

	/**
	 * \brief Configure simulation of an ns-3  application
	 * \param argc program arguments count
	 * \param argv program arguments
	 * \return none
	 */
	void Configure (int argc, char *argv[]);

	/**
	 * \brief Enacts simulation of an ns-3  application
	 * \return none
	 */
	void Simulate ();

	/**
	 * \brief Process outputs
	 * \return none
	 */
	void ProcessOutputs ();

	/**
	 * \brief Calculates the filepath where the output of the experiment will be saved (depends on input parameters)
	 * \return file path
	 */
	const std::string CalculateOutFilePath() const;


	/**
	 * \brief Calculates configure, runs and print results of the experiment
	 * \return file path
	 */
	void RunAndPrintResults(int argc, char *argv[]);

protected:
	/**
	 * \brief Process command line arguments
	 * \param argc program arguments count
	 * \param argv program arguments
	 * \return none
	 */
	void ParseCommandLineArguments (int argc, char *argv[]);

	/**
	 * \brief Configure default attributes
	 * \return none
	 */
	void ConfigureDefaults ();

	/**
	 * \brief Configure nodes
	 * \return none
	 */
	void ConfigureNodes ();

	/**
	 * \brief Configure devices
	 * \return none
	 */
	void ConfigureDevices ();

	/**
	 * \brief Configure mobility
	 * \return none
	 */
	void ConfigureMobility ();

	/**
	 * \brief Set up the adhoc devices
	 * \return none
	 */
	void SetupAdhocDevices ();

	/**
	 * \brief Configure connections
	 * \return none
	 */
	void ConfigureConnections ();

	/**
	 * \brief Configure tracing and logging
	 * \return none
	 */
	void ConfigureTracingAndLogging ();

	/**
	 * \brief Configure the FB application
	 * \return none
	 */
	void ConfigureFBApplication ();

	/**
	 * \brief Run the simulation
	 * \return none
	 */
	void RunSimulation ();

private:
	/**
	 * \brief Run the simulation
	 * \return none
	 */
	void Run ();

	/**
	 * \brief Run the simulation
	 * \return none
	 */
	void CommandSetup (int argc, char *argv[]);

	/**
	 * \brief Set up a prescribed scenario
	 * \return none
	 */
	void SetupScenario ();

	/**
	 * \brief Set up receivers socket
	 * \param node node to configure
	 * \return socket created
	 */
	Ptr<Socket> SetupPacketReceive (Ptr<Node> node);

	/**
	 * \brief Set up senders socket
	 * \param addr address of the node
	 * \param node node to configure
	 * \return socket created
	 */
	Ptr<Socket> SetupPacketSend (Ipv4Address addr, Ptr<Node> node);

	/**
	* \brief Calculates number of nodes based on .ns2mobility filepath
	* \return Number of nodes (vehicles) in simulation
	*/
	unsigned int CalculateNumNodes() const;

	/**
   * \brief Prints actual position and velocity when a course change event occurs
   * \return none
   */
	static void
	CourseChange (std::ostream *os, std::string foo, Ptr<const MobilityModel> mobility);


	Ptr<FBApplication>						m_fbApplication;
	uint32_t 								m_nNodes;
	NodeContainer							m_adhocNodes;
	Ptr<ListPositionAllocator> 				m_adhocPositionAllocator;
	NetDeviceContainer						m_adhocDevices;
	Ipv4InterfaceContainer					m_adhocInterfaces;
	std::vector <Ptr<Socket>>				m_adhocSources;
	std::vector <Ptr<Socket>>				m_adhocSinks;
	std::string								m_packetSize;
	std::string								m_rate;
	std::string								m_phyMode;
	double									m_txp;
	uint32_t								m_port;
	uint32_t								m_actualRange;
	uint32_t								m_startingNode;
	uint32_t								m_staticProtocol;
	uint32_t								m_flooding;
	uint32_t								m_alertGeneration;
	uint32_t								m_areaOfInterest;
	uint32_t								m_vehicleDistance;
	uint32_t								m_scenario;
	uint32_t								m_loadBuildings;
	std::string								m_traceFile;
	std::string								m_bldgFile;
	std::string								m_mapBasePath;
	std::string								m_mapBaseName;
	std::string									m_mapBaseNameWithoutDistance;
	double									m_TotalSimTime;


};

/* -----------------------------------------------------------------------------
*			METHOD DEFINITIONS
* ------------------------------------------------------------------------------
*/

FBVanetExperiment::FBVanetExperiment ()
	:	m_nNodes (0),	// random value, it will be set later
		m_packetSize ("64"),
		m_rate ("2048bps"),
		m_phyMode ("DsssRate11Mbps"),
		m_txp (20),
		m_port (9),
		m_actualRange (300),
		m_startingNode (0),
		m_staticProtocol (1),
		m_flooding (0),
		m_alertGeneration (20),
		m_areaOfInterest (1000),
		m_vehicleDistance (250),
		m_scenario (1),
		m_loadBuildings (1),
		m_traceFile (""),
		m_bldgFile (""),
		m_TotalSimTime (30)
{
	srand (time (0));

	RngSeedManager::SetSeed (time (0));
}

FBVanetExperiment::~FBVanetExperiment ()
{
}

void
FBVanetExperiment::Configure (int argc, char *argv[])
{
	// Initial configuration and parameters parsing
	ParseCommandLineArguments (argc, argv);
	ConfigureDefaults ();

	ConfigureTracingAndLogging ();
}

void
FBVanetExperiment::Simulate ()
{
	// Configure the network and all the elements in it
	ConfigureNodes ();
	ConfigureMobility ();
	SetupAdhocDevices ();
	ConfigureConnections ();

	ConfigureFBApplication ();

	// Run simulation and print some results
	RunSimulation ();
}

void
FBVanetExperiment::ProcessOutputs ()
{
	NS_LOG_FUNCTION (this);
	NS_LOG_INFO ("Process outputs.");

	std::stringstream dataStream;
	m_fbApplication->PrintStats (dataStream);

	g_csvData.AddValue((int) m_scenario);
	g_csvData.AddValue((int) m_actualRange);
	g_csvData.AddValue((int) m_staticProtocol);
	g_csvData.AddValue((int) m_loadBuildings);
	g_csvData.AddValue((int) m_nNodes);
	g_csvData.AddMultipleValues(dataStream);
	g_csvData.CloseRow ();
}

const std::string
FBVanetExperiment::CalculateOutFilePath() const {
	std::string fileName = "";
	std::string vehicleDistance = std::to_string(m_vehicleDistance);
	std::string buildings = std::to_string(m_loadBuildings);
	std::string protocol = "";
	std::string actualRange = std::to_string(m_actualRange);


	if (m_staticProtocol == PROTOCOL_FB) {
		protocol = "fb";
	}
	else if (m_staticProtocol == PROTOCOL_STATIC_100) {
		protocol = "st100";
	}
	else if (m_staticProtocol == PROTOCOL_STATIC_300) {
		protocol = "st300";
	}
	else if (m_staticProtocol == PROTOCOL_STATIC_500) {
		protocol = "st500";
	}

	fileName.append(m_mapBaseNameWithoutDistance + "/d" + vehicleDistance + "/b" + buildings + "/" + protocol + "-" + actualRange + "/"
			+ m_mapBaseName + "-b" + buildings + "-" + protocol + "-" + actualRange);

	return fileName;
}

void
FBVanetExperiment::RunAndPrintResults(int argc, char *argv[]) {
	Configure(argc, argv);
	Simulate();
	ProcessOutputs();
}

void
FBVanetExperiment::ConfigureDefaults ()
{
	NS_LOG_FUNCTION (this);

	Config::SetDefault ("ns3::OnOffApplication::PacketSize",StringValue (m_packetSize));
	Config::SetDefault ("ns3::OnOffApplication::DataRate",  StringValue (m_rate));
	Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (m_phyMode));

	if (m_staticProtocol == 1)
		m_staticProtocol = PROTOCOL_FB;
	else if (m_staticProtocol == 2)
		m_staticProtocol = PROTOCOL_STATIC_100;
	else if (m_staticProtocol == 3)
		m_staticProtocol = PROTOCOL_STATIC_300;
	else if (m_staticProtocol == 4)
		m_staticProtocol = PROTOCOL_STATIC_500;
}

void
FBVanetExperiment::ParseCommandLineArguments (int argc, char *argv[])
{
	NS_LOG_FUNCTION (this);

	CommandSetup (argc, argv);
	SetupScenario ();
}

void
FBVanetExperiment::ConfigureNodes ()
{
	NS_LOG_FUNCTION (this);
	NS_LOG_INFO ("Setup nodes.");

	m_adhocNodes.Create (m_nNodes);
}

void
FBVanetExperiment::ConfigureMobility ()
{
	NS_LOG_FUNCTION (this);
	NS_LOG_INFO ("Configure current mobility mode.");

	// Create Ns2MobilityHelper with the specified trace log file as parameter
	Ns2MobilityHelper ns2 = Ns2MobilityHelper (m_traceFile);
	NS_LOG_INFO ("Loading ns2 mobility file \"" << m_traceFile << "\".");

	// Disable node movements
	ns2.DisableNodeMovements ();

	ns2.Install (); // configure movements for each node, while reading trace file

	// Configure callback for logging
	std::ofstream m_os;
	Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
									 MakeBoundCallback (&FBVanetExperiment::CourseChange, &m_os));
}

void
FBVanetExperiment::SetupAdhocDevices ()
{
	NS_LOG_FUNCTION (this);
	NS_LOG_INFO ("Configure channels.");

	double freq = 2.4e9;	// 802.11b 2.4 GHz

	WifiHelper wifi;
	wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

	YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
	YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel", "Frequency", DoubleValue (freq), "HeightAboveZ", DoubleValue (1.5));
	if (m_loadBuildings != 0)
	{
		wifiChannel.AddPropagationLoss ("ns3::ObstacleShadowingPropagationLossModel", "Radius", DoubleValue (200));
	}
	wifiPhy.SetChannel (wifiChannel.Create ());
	wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);

	if (m_actualRange == 100)
		m_txp = -7.0;
	else if (m_actualRange == 300)
		m_txp = 4.6;
	else if (m_actualRange == 500)
		m_txp = 13.4;

	WifiMacHelper wifiMac;
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
																"DataMode",StringValue (m_phyMode),
																"ControlMode",StringValue (m_phyMode));
	wifiPhy.Set ("TxPowerStart",DoubleValue (m_txp));
	wifiPhy.Set ("TxPowerEnd", DoubleValue (m_txp));

	// wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue ());
	wifiMac.SetType ("ns3::AdhocWifiMac");

	m_adhocDevices = wifi.Install (wifiPhy, wifiMac, m_adhocNodes);
}

void
FBVanetExperiment::ConfigureConnections ()
{
	NS_LOG_FUNCTION (this);
	NS_LOG_INFO ("Configure connections.");

	InternetStackHelper internet;
	internet.Install (m_adhocNodes);

	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.1.0.0", "255.255.0.0");
	m_adhocInterfaces = ipv4.Assign (m_adhocDevices);

	// TODO
	OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address ());
	onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
	onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

	// Set receiver (for each node in the application)
	for (uint32_t i = 0; i < m_nNodes; i++)
	{
		SetupPacketReceive (m_adhocNodes.Get (i));
		AddressValue remoteAddress (InetSocketAddress (ns3::Ipv4Address::GetAny (), m_port));
		onoff1.SetAttribute ("Remote", remoteAddress);
	}

	// Set unicast sender (for each node in the application)
	for (uint32_t i = 0; i < m_nNodes; i++)
	{
		SetupPacketSend (ns3::Ipv4Address("10.1.255.255"),  m_adhocNodes.Get (i));
	}
}

void
FBVanetExperiment::ConfigureTracingAndLogging ()
{
	NS_LOG_FUNCTION (this);

	Packet::EnablePrinting ();
}

void
FBVanetExperiment::ConfigureFBApplication ()
{
	NS_LOG_FUNCTION (this);
	NS_LOG_INFO ("Configure FB application.");

	// Delete pre-existing application
	if (m_fbApplication)
		m_fbApplication = 0;

	// Create the application and schedule start and end time
	m_fbApplication = CreateObject<FBApplication> ();
	m_fbApplication->Install (m_staticProtocol,
														m_alertGeneration,
														m_actualRange,
														m_areaOfInterest,
														m_vehicleDistance,
														(m_flooding==1) ? true : false,
														32, 1024);
	m_fbApplication->SetStartTime (Seconds (1));
	m_fbApplication->SetStopTime (Seconds (m_TotalSimTime));

	// Add nodes to the application
	for (uint32_t i = 0; i < m_nNodes; i++)
	{
		m_fbApplication->AddNode (m_adhocNodes.Get (i), m_adhocSources.at (i), m_adhocSinks.at (i), true);
	}

	// Add the application to a node
	m_adhocNodes.Get (m_startingNode)->AddApplication (m_fbApplication);
}

void
FBVanetExperiment::RunSimulation ()
{
	NS_LOG_FUNCTION (this);
	NS_LOG_INFO ("Run simulation.");

	Run ();
}

void
FBVanetExperiment::CommandSetup (int argc, char *argv[])
{
	NS_LOG_FUNCTION (this);
	NS_LOG_INFO ("Parsing command line arguments.");

	CommandLine cmd;

	// allow command line overrides
//	cmd.AddValue ("nnodes", "Number of nodes (i.e. vehicles)", m_nNodes);
//	cmd.AddValue ("nodes", "Id of the first node who will start an aler", m_startingNode);
	cmd.AddValue ("actualRange", "Actual transimision range (meters)", m_actualRange);
	cmd.AddValue ("protocol", "Estimantion protocol: 1=FB, 2=C100, 3=C300, 4=C500", m_staticProtocol);
	cmd.AddValue ("flooding", "Enable flooding", m_flooding);
	cmd.AddValue ("alertGeneration", "Time at which the first Alert Message should be generated.", m_alertGeneration);
	cmd.AddValue ("area", "Radius of the area of interest", m_areaOfInterest);
//	cmd.AddValue ("scenario", "1=Padova, 2=Los Angeles", m_scenario);
	cmd.AddValue ("buildings", "Load building (obstacles)", m_loadBuildings);
	cmd.AddValue ("trace", "Vehicles trace file (ns2mobility format)", m_traceFile);
	cmd.AddValue ("totalTime", "Simulation end time", m_TotalSimTime);

	cmd.AddValue ("mapBasePath", "Base path of map required for simulation "
			"(e.g. ../maps/Padova-25.osm.xml. The dash '-' in the name is mandatory)", m_mapBasePath);

	cmd.Parse (argc, argv);
}

void
FBVanetExperiment::SetupScenario ()
{
	NS_LOG_FUNCTION (this);
//	NS_LOG_INFO ("Configure current scenario (" << m_scenario << ").");

	m_alertGeneration = 9;	// 10 -1 (start time of the application)
	m_TotalSimTime = 990000.0;
	m_areaOfInterest = 1000;	// meters

	// Calculates file base name and vehicle distance from file base path
	size_t foundSlash = m_mapBasePath.find_last_of("/\\");
	size_t foundDash = m_mapBasePath.find_last_of("-");

	m_mapBaseName = m_mapBasePath.substr(foundSlash + 1);
	m_mapBaseNameWithoutDistance = m_mapBaseName.substr(0, m_mapBaseName.find_last_of("-"));
	m_vehicleDistance = std::stoi(m_mapBasePath.substr(foundDash + 1));

	m_bldgFile = m_mapBasePath + ".poly.xml";
	m_traceFile = m_mapBasePath + ".ns2mobility.xml";
	m_nNodes = CalculateNumNodes();
	m_startingNode = rand() % m_nNodes;
	cout << "numNodes = " << m_nNodes << endl;
	cout << "startingNode = " << m_startingNode << endl;
//
//	if (m_scenario == 0)
//	{
//		// DEBUG, TODO: delete this scenario
//		m_bldgFile = "Griglia.poly.xml";
//		m_traceFile = "Griglia.ns2mobility.xml";
//
//		m_nNodes = 96;
//		m_startingNode = 23;
//
//		m_areaOfInterest = 500;
//	} else if (m_scenario == 1)
//	{
//		// Padova (2x2 km)
//		m_bldgFile = "Padova.poly.xml";
//		m_traceFile = "Padova.ns2mobility.xml";
//
//		m_nNodes = 2224;
//		m_startingNode = 313;
//	}
//	else if (m_scenario == 2)
//	{
//		// L.A.  (2x2 km)
//		m_bldgFile = "LA.poly.xml";
//		m_traceFile = "LA.ns2mobility.xml";
//
//		m_nNodes = 1905;
//		m_startingNode = 365;
//	}
//	else
//		NS_LOG_ERROR ("Invalid scenario specified. Values must be [1-2].");

	if (m_loadBuildings != 0)
	{
		NS_LOG_INFO ("Loading buildings file \"" << m_bldgFile << "\".");
		Topology::LoadBuildings (m_bldgFile);
	}
}

unsigned int
FBVanetExperiment::CalculateNumNodes() const {
	NS_LOG_FUNCTION (this);
	ifstream ns2mobilityTraceFile;
	ns2mobilityTraceFile.open(m_traceFile, ios::in);
	if (!ns2mobilityTraceFile.is_open()) {
		NS_LOG_ERROR("Could not open ns2MobilityTraceFile");
	}
	string line;
	unsigned int numNodes = 0;
	while(getline(ns2mobilityTraceFile, line)) {
		vector<string> strings;
		boost::split(strings, line, boost::is_any_of(" "));
		if (strings.size() == 4) {
			string str = strings.at(0);
			unsigned int openParensPos = str.find_first_of("(");
			unsigned int closeParensPos = str.find_first_of(")");
			unsigned int numNodeCandidate = stoi(str.substr(openParensPos + 1, closeParensPos - openParensPos - 1));
			if (numNodeCandidate > numNodes) {
				numNodes = numNodeCandidate;
			}
		}
	}
	return numNodes;
}

void
FBVanetExperiment::Run ()
{
	NS_LOG_FUNCTION (this);

	Simulator::Stop (Seconds (m_TotalSimTime));
//	AnimationInterface anim ("animation.xml");
	Simulator::Run ();

	Simulator::Destroy ();
}

void
FBVanetExperiment::CourseChange (std::ostream *os, std::string foo, Ptr<const MobilityModel> mobility)
{
	NS_LOG_FUNCTION ( &os << foo << mobility);	// problem with the argument *os

  Vector pos = mobility->GetPosition (); // Get position
  Vector vel = mobility->GetVelocity (); // Get velocity

  int nodeId = mobility->GetObject<Node> ()->GetId ();

  NS_LOG_DEBUG ("Changing pos for node " << nodeId << " at " << Simulator::Now ().GetSeconds ()
	 							<< "; POS: (" << pos.x << ", " << pos.y << ", " << pos.z << ")"
								<< "; VEL: (" << vel.x << ", " << vel.y << ", " << vel.z << ").");
}

Ptr<Socket>
FBVanetExperiment::SetupPacketReceive (Ptr<Node> node)
{
	NS_LOG_FUNCTION (this << node);

	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	Ptr<Socket> sink = Socket::CreateSocket (node, tid);
	InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
	sink->Bind (local);

	// Store socket
	m_adhocSinks.push_back (sink);

	return sink;
}

Ptr<Socket>
FBVanetExperiment::SetupPacketSend (Ipv4Address addr, Ptr<Node> node)
{
	NS_LOG_FUNCTION (this << addr << node);

	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	Ptr<Socket> sender = Socket::CreateSocket (node, tid);
	InetSocketAddress remote = InetSocketAddress (addr, m_port);
	sender->SetAllowBroadcast (true);
	sender->Connect (remote);

	// Store socket
	m_adhocSources.push_back (sender);

	return sender;
}


/* -----------------------------------------------------------------------------
*			MAIN
* ------------------------------------------------------------------------------
*/

void runExperiment(int argc, char *argv[], unsigned int minId, unsigned int maxId) {
	cout << "runExperiment" << endl;
	for (unsigned int i = minId; i < maxId; i++) {
		cout << "run = " << i << endl;
		FBVanetExperiment experiment;
		experiment.RunAndPrintResults(argc, argv);
	}
}

int main (int argc, char *argv[])
{
    cout << "Start main urban" << endl;
	NS_LOG_UNCOND ("FB Vanet Experiment URBAN");
	unsigned int maxRun = RngSeedManager::GetRun();

//	Before launching experiments, calculate output file path
	FBVanetExperiment experiment;
	experiment.Configure (argc, argv);
	std::string filePath = experiment.CalculateOutFilePath();
	boost::filesystem::path path = boost::filesystem::current_path() /= "/out/scenario-urbano/";
	path /= filePath;
	g_csvData.EnableAlternativeFilename (path);
	g_csvData.WriteHeader ("\"id\",\"Scenario\",\"Actual Range\",\"Protocol\",\"Buildings\",\"Total nodes\","
							"\"Nodes on circ\",\"Total coverage\",\"Coverage on circ\",\"Alert received mean time\",\"Hops\","
							"\"Slots\",\"Messages sent\",\"Messages received\"");
	for(unsigned int i = 0; i < maxRun; i++) {
		cout << "run = " << i << endl;
		FBVanetExperiment experiment;
		experiment.RunAndPrintResults(argc, argv);
	}

}
