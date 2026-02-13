
#include "Printer.h"

Printer::Printer(ParsedBuffers &pb, Crane &crane) : pb(&pb), crane(&crane){
    NArrivals = getNumberOfStacksInSameCategory('A');
    NBuffers = getNumberOfStacksInSameCategory('B');
    NHoldovers = getNumberOfStacksInSameCategory('H');
}

Printer::~Printer() {}

int Printer::getNumberOfStacksInSameCategory(char c){
    int res = 0;
    for(auto& stack : pb->getBuffers()){
        if(stack->getName()[0]==c){
            res++;
        }
    }
    return res;
}

bool Printer::sameCategoryStack(const std::string &stack1,const std::string &stack2){
    if(stack1.empty() || stack2.empty()){
        return false;
    }
    if(stack1[0]==stack2[0]){
        return true;
    }
    return false;
}

int Printer::calculateIndexOfHook(){
    bool A=0;
    bool B=0;
    bool H=0;
    std::string iznadStoga = crane->getAboveStack(); //pozvati f-ju od kuke koja ce vratiti naziv stoga

    switch (iznadStoga[0])
    {
    case 'A':
        A=1;
        break;
    case 'B':
        B=1;
        break;  
    default:
        H=1;
        break;
    }
    int temp = iznadStoga[1]-'0';
    //uvijek trebam sredisnji broj iznad stoga => 1.stog indeks=5
    if(A){
        return (temp+1)*11 + temp - 6;
    }
    if(B){
        return NArrivals*11 + 3 + (temp+1)*11+temp - 6;
    }
    return NArrivals*11+NArrivals+3 + NBuffers*11+NBuffers+3 + (temp+1)*11+temp - 6;
}

int Printer::calculateHeight(){
    int maxSize=13;
    int maxOccupancy=0;
    std::vector<Buffer*> buffers = pb->getBuffers();
    for(auto& stack : pb->getBuffers()){
        if(maxOccupancy<stack->stackOccupancy()){
            maxOccupancy = stack->stackOccupancy();
        }
        if (dynamic_cast<Buffer*>(stack)) {
            if(maxSize<stack->getSize()){
                maxSize = stack->getSize();
            }
        }
    }
    return maxSize - maxOccupancy;
}

std::string Printer::centerText(const std::string& text, int width) {
    if (width <= static_cast<int>(text.size())) {
        return text; // If the text is longer than or equal to the width, return as is
    }
    int padding = width - text.size();
    int leftPadding = padding / 2;
    int rightPadding = padding - leftPadding;
    return std::string(leftPadding, ' ') + text + std::string(rightPadding, ' ');
}

void Printer::printEverything(){
    std::vector<Buffer*> buffers = pb->getBuffers();
    std::vector<Buffer*> tempStacks;
    for(auto &stack : buffers){
        Buffer* nb = new Buffer(stack->getSize(),stack->getName());
        for(auto &container : stack->getContainers()){
            nb->push(*container);   
        }
        tempStacks.push_back(nb);
    }
    int maxStackHeight = 0;
    for (const auto& stack : buffers) {
        maxStackHeight = std::max(maxStackHeight, stack->stackOccupancy());
    }
    int HookIndex = calculateIndexOfHook();
    int height=calculateHeight();    //->koliko ce redova bit od kuke do dna stogova, potrebno radi zadrzavanja visine "prozora"
    int width=buffers.size()*11 + buffers.size() + 4; //sirina cijelog "prozora", 9=ona 3 puta po 3 razmaka

    //kuka dio
    for(int i=0;i<width;i++){
        if(i==HookIndex){
            std::cout<<"|";
        }
        else{
            std::cout<<"-";
        }
    }
    std::cout<<std::endl;
    for(int i=0;i<width;i++){
        if(i==HookIndex-1){
            std::cout<<"[ ]";
        }
        else{
            std::cout<<" ";
        }
    }
    std::cout<<std::endl; 
    for(int i=0;i<height;i++){
        std::cout<<std::endl;
    }

    //stogovi dio
    ContainerStack* prev = NULL;
    for(int currentHeightOfStack = maxStackHeight;currentHeightOfStack>0;currentHeightOfStack--){
        //prolazit po buffers i gledat koliki je size svakog stoga
        // -> ako je jednak chs onda popat element i ispisat ga
        // -> ako nije onda stavit " "
        prev = NULL;
        for(auto& stack : tempStacks){
            if(prev!=NULL && !sameCategoryStack(prev->getName(),stack->getName())){
                std::cout<<"   ";
            }
            else if(prev!=NULL){
                std::cout<<" ";
            }
            prev = stack;
            if(currentHeightOfStack == stack->stackOccupancy()){
                Container* el = stack->pop();
                std::string temp = el->getDetails();
                temp = centerText(temp,11);
                std::cout<<temp;
            }
            else{
                std::string temp = centerText(" ",11);
                std::cout<<temp;
            }
        }
        std::cout<<std::endl;
   }

   //ispis dna i naziva pojedinog stoga
    prev = NULL;
    for(auto& stack : buffers){
        if(prev!=NULL && !sameCategoryStack(prev->getName(),stack->getName())){
            std::cout<<"   ";
        }
        else if(prev!=NULL){
            std::cout<<" ";
        }
        prev = stack;
        for(int i=0;i<11;i++){
            std::cout<<"-";
        }
    }
    std::cout<<std::endl;

    prev = NULL;
    for(auto& stack : buffers){
        if(prev!=NULL && !sameCategoryStack(prev->getName(),stack->getName())){
            std::cout<<"   ";
        }
        else if(prev!=NULL){
            std::cout<<" ";
        }
        prev = stack;
        std::ostringstream oss;
        oss << stack->getName() << '(' << stack->stackOccupancy() << ')';
        std::string temp = oss.str();
        temp = centerText(temp,11);
        std::cout<<temp;
    }
    std::cout<<std::endl;
    std::cout<<std::endl;
}

ParsedBuffers* Printer::getParsedBuffers(){
    return pb;
}

Crane* Printer::getCrane(){
    return crane;
}