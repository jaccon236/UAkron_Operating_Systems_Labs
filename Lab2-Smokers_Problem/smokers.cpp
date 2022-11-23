/*
    COPYRIGHT (C) 2021 Jacen Conlan (jcc126) All rights reserved.

    Operating Systems Concepts - 3460:426
    Lab 2 - POSIX Threads and Semaphores
    Filename: smokers.cpp
    Author: Jacen Conlan (jcc126@zips.uakron.edu)
    Version: 1.01 03.21.2021

    To compile this program run the command "g++ -pthread smokers.cpp" in the
    same directory which contains this program
*/

#include <iostream>
#include <iomanip>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mutex>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define GRAY "\033[0;37m"
#define GREEN "\033[0;32m"
#define BRIGHT_RED "\033[0;91m"
#define MAGENTA "\033[0;95m"
#define YELLOW "\033[0;33m"
#define CYAN "\033[0;36m"
#define RESET "\033[0m"

using namespace std;

// -----------------------Semaphore Class Declaration--------------------------
class Semaphore
{
    private:
        sem_t sem;

    public:
        // Default Constructor
        Semaphore() { sem_init(&sem, 0, 0); }

        // Initialized Constructor
        Semaphore(int status) { sem_init(&sem, 0, status); }

        // Invalid Constructors (throws error if called)
        Semaphore(Semaphore const&) = delete;
        Semaphore& operator=(Semaphore const&) = delete;

        // Destructor
        ~Semaphore() { sem_destroy(&sem); }

        // Member Functions
        int wait() { return sem_wait(&sem); }
        int signal() { return sem_post(&sem); }
};

// -------------------------Function Declarations------------------------------
void* agent(void*);
void* pusher(void*);
void* smoker(void*);

// ----------------------------Global Variables--------------------------------
// Items on table = {Tobacco, Paper, Match}
bool isOnTable[3] = {false, false, false};

// Smokers' Haves and Needs
string hadItems[3] = {"tobacco", "paper", "a match"};
string neededItems[3] = {"paper and a match", "tobacco and a match",
                          "tobacco and paper"};
// Semaphore regarding the Agents' and Pushers' Status
//      0 = Busy
//      1 = Ready
Semaphore agentStatus(1);
Semaphore pusherStatus(1);

// Mutex lock for cout usage
//      call coutStatus.lock() before calling cout
//      call coutStatus.unlock() after calling cout
mutex coutStatus;

// Arrays for types of Smokers and Pushers
Semaphore smokers[3];
Semaphore pushers[3];

// -----------------------Smoker's Problem Solution----------------------------
int main()
{
    // Seed the random function
    srand(time(NULL));

    // Allocate 6 smokerIDs and 6 smokerThreads
    int smokerIDs[6];
    pthread_t smokerThreads[6];

    // Initialize smokerIDs and create smokerThreads
    for (int i = 0; i < 6; i++)
    {
        smokerIDs[i] = i;

        // If thread creation fails, exit gracefully
        if (pthread_create(&smokerThreads[i], NULL, smoker, &smokerIDs[i]))
        {
            cout << "Unable to create smoker thread!" << endl;
            exit(1);
        }
    }

    // Allocate 3 pusherIDs and 3 pusherThreads
    int pusherIDs[3];
    pthread_t pusherThreads[3];

    // Initialize pusherIDs and create pusherThreads
    for (int i = 0; i < 3; i++)
    {
        pusherIDs[i] = i;

        // If thread creation fails, exit gracefully
        if (pthread_create(&pusherThreads[i], NULL, pusher, &pusherIDs[i]))
        {
            cout << "Unable to create pusher thread!" << endl;
            exit(1);
        }
    }

    // Allocate 3 agentIDs and 3 agentThreads
    int agentIDs[3];
    pthread_t agentThreads[3];

    // Initialize agentIDs and create agentThreads
    for (int i = 0; i < 3; i++)
    {
        agentIDs[i] = i;

        // If thread creation fails, exit gracefully
        if (pthread_create(&agentThreads[i], NULL, agent, &agentIDs[i]))
        {
            cout << "Unable to create agent thread!" << endl;
            exit(1);
        }
    }

    // Join smokerThreads for program completion
    for (int i = 0; i < 6; i++)
    {
        pthread_join(smokerThreads[i], NULL);
    }

    // Join pusherThreads for program completion
    for (int i = 0; i < 3; i++)
    {
        pthread_join(pusherThreads[i], NULL);
    }

    // Join agentThreads for program completion
    for (int i = 0; i < 3; i++)
    {
        pthread_join(agentThreads[i], NULL);
    }

    cout << RESET << endl;
    return 0;
}

// ---------------------------Function Definitions-----------------------------
// Agent Thread Function
void* agent(void* arg)
{
    int agentID = *(int*) arg;

    // Run each agent 6 times
    for (int i = 0; i < 6; i++)
    {
        // Sleep for a random time between 0ms and 200ms
        sleep((rand() % 200) / 1000);

        // Set agentStatus to busy and wait until signaled
        agentStatus.wait();

        // Lock coutStatus then cout a message including the items that
        // "Agent" is putting on the table. Then unlock coutStatus.
        coutStatus.lock();
        cout << MAGENTA << "Agent " << agentID << " is putting "
             << neededItems[agentID] << " on the table." << endl;
        coutStatus.unlock();


        // Signal the pushers that tell smokers what items are on the table
        pushers[(agentID + 1) % 3].signal();
        pushers[(agentID + 2) % 3].signal();
    }

    return NULL;
}

// Pusher Thread Function
void* pusher(void* arg)
{
    int pusherID = *(int*) arg;

    // Run each pusher 12 times
    for (int i = 0; i < 12; i++)
    {
        // Set current pusher and pusherStatus to busy and wait until signaled
        pushers[pusherID].wait();
        pusherStatus.wait();

        // Check to see if another pusher has already revealed an item
        // An item differing from this pusher's item was already revealed
        if(isOnTable[(pusherID + 1) % 3])
        {
            // Lock coutStatus then cout a message showing what item has already
            // been revealed and what item is now being revealed.
            // Then unlock coutStatus.
            coutStatus.lock();
            cout << YELLOW << "Pusher " << pusherID << ": A previous pusher "
                 << "already revealed " << hadItems[(pusherID + 1) % 3]
                 << ". Revealing " << hadItems[pusherID] << " on the table now."
                 << endl;
            coutStatus.unlock();
            coutStatus.unlock();

            // Reset revealed items for the next agent and pushers
            isOnTable[(pusherID + 1) % 3] = false;

            // Signal the smoker that needs the items that are on the table
            smokers[(pusherID + 2) % 3].signal();
        }

        // An item differing from this pusher's item was already revealed
        else if(isOnTable[(pusherID + 2) % 3])
        {
            // Lock coutStatus then cout a message showing what item has already
            // been revealed and what item is now being revealed.
            // Then unlock coutStatus.
            coutStatus.lock();
            cout << YELLOW << "Pusher " << pusherID << ": A previous pusher "
                 << "already revealed " << hadItems[(pusherID + 2) % 3]
                 << ". Revealing " << hadItems[pusherID] << " on the table now."
                 << endl;
            coutStatus.unlock();


            // Reset revealed items for the next agent and pushers
            isOnTable[(pusherID + 2) % 3] = false;

            // Signal the smoker that needs the items that are on the table
            smokers[(pusherID + 1) % 3].signal();
        }

        // No items have been revealed
        else
        {
            // Lock coutStatus then cout a message revealing the first item on
            // the table. Then unlock coutStatus.
            coutStatus.lock();
            cout << YELLOW << "Pusher " << pusherID << ": " << "Nothing has "
                 << "been revealed yet. Revealing " << hadItems[pusherID]
                 << " on the table now." << endl;
            coutStatus.unlock();

            // Set the boolean flag for this pusher's item as revealed
            isOnTable[pusherID] = true;
        }

        // Signal that pusherStatus is ready for the next pusher
        pusherStatus.signal();
    }

    return NULL;
}

// Smoker Thread Function
void* smoker(void* arg)
{
    int smokerID = *(int*) arg;

    // Limit to 3 types of smokers
    int typeOfSmoker = smokerID % 3;

    // Each smoker smokes 3 cigarettes before becoming hungry
    for (int i = 0; i < 3; i++)
    {
        // Lock coutStatus then cout a message that the current smoker has one
        // item and is looking for two more items to make their cigarette.
        // Then unlock coutStatus.
        coutStatus.lock();
        cout << GRAY << "Smoker " << smokerID << " has "
             << hadItems[typeOfSmoker] << " and needs "
             << neededItems[typeOfSmoker] << "." << endl;
        coutStatus.unlock();

        // Wait until this smoker is signaled that they can make a cigarette
        smokers[typeOfSmoker].wait();

        // lock coutStatus then cout a message stating that the current smoker
        // is making a cigarette. Then unlock coutStatus.
        coutStatus.lock();
        cout << CYAN << "Smoker " << smokerID
             << " is now making a cigarette." << endl;
        coutStatus.unlock();

        // Spend 50ms making a cigarette
        sleep(0.05);

        // Signal agents that the table is free for more items
        agentStatus.signal();

        // Lock coutStatus then cout a message stating that the current smoker
        // is smoking their cigarette. Then unlock coutStatus.
        coutStatus.lock();
        cout << GREEN << "Smoker " << smokerID
             << " is smoking their cigarette." << endl;
        coutStatus.unlock();

        // Spend 50ms smoking a cigarette
        sleep(0.05);
    }

    // When the current smoker has finished their third cigarette they will
    // complain that the extremely unhealthy habit of chain smoking has made
    // them extremely hungry. Don't forget to lock and unlock coutStatus.
    coutStatus.lock();
    cout << BRIGHT_RED << "Smoker " << smokerID << ": " << BRIGHT_RED
         << "Oh no! This horrible habit of smoking these disgusting "
         << "cigarettes has made me extremely hungry!" << endl;
    coutStatus.unlock();

    return NULL;
}

/*
END OF FILE
*/
