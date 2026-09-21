#include <iostream>
using namespace std;

//node structure
struct Node{
  string name;
  int age;
  Node* next;
};

//the linked list class(manages the nodes)
class LinkedList{
private:
  Node* head; //pointer to first node

public:
  LinkedList(){
    head = nullptr; //EmptyList
  }

  void addNode(string name, int age){
    Node* newNode = new Node;
    newNode->name = name;
    newNode->age = age;
    newNode->next = head; //new Node points
    head = newNode; //head now points
  }
  void display(){
    Node* current = head;
    while(current !=nullptr){
      cout<<"Name: "<<current->name
        <<"age: "<<current->age<<endl;
      current = current->next; //move to next node
    }

  }
};
int main(){
  LinkedList list;

  list.addNode("Alice", 25);
  list.addNode("Bob",24);
  list.addNode("char", 25);

  list.display();
  return 0;
}
