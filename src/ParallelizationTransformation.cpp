//===------------ ParallelizationTransformation.cpp ParallelizationTransformation -----------===//
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implmentation of the ParallelizationTransformation class, which
/// contains the declartion of the Parallelization transformation
///
//===----------------------------------------------------------------------===//
#include "ParallelizationTransformation.h"
#include "mlir/Dialect/Transform/Utils/DiagnosedSilenceableFailure.h"
//#include "/scratch/ia2280/LLVM/llvm-project/mlir/lib/Dialect/Linalg/TransformOps/LinalgTransformOps.cpp";
#pragma once
//#include "/scratch/ia2280/LLVM/llvm-project/mlir/lib/Dialect/Linalg/TransformOps/LinalgTransformOps.cpp";
using namespace mlir;
void generateCombinations(const llvm::SmallVector<llvm::SmallVector<int64_t, 4>, 4> &tileSizes,
                          int64_t maxNumberLoops,
                          int64_t currentLoop,
                          SmallVector<int64_t, 4> &currentCombination,
                          std::vector<SmallVector<int64_t, 4>> &combinations);

llvm::SmallVector<llvm::SmallVector<int64_t, 4>, 4>
generateTileForAllOpCombinations(int64_t maxNumberLoops,
                              const llvm::SmallVector<llvm::SmallVector<int64_t, 4>, 4> &possibleTileSizes,
                              const llvm::SmallVector<int64_t> &upperBounds);
/*void generateForAllOpCombinations(const SmallVector<int64_t, 4> &tileSizes,
                                  int64_t maxNumberLoops,
                                  int64_t currentLoop,
                                  SmallVector<int64_t, 4> &currentCombination,
                                  std::vector<SmallVector<int64_t, 4>> &combinations);
SmallVector<SmallVector<int64_t, 4>, 4>
generateTileForAllOpCombinations(int64_t maxNumberLoops,
                                 const std::vector<int64_t> &possibleTileSizes);*/



Parallelization::Parallelization(mlir::TilingInterface *op,
                                 llvm::SmallVector<int64_t, 4> tileSizes,
                                 mlir::MLIRContext *context)
{
  this->op = op;
  this->context = context;
  this->tileSizes = tileSizes;
}

llvm::SmallVector<int64_t, 4> Parallelization::getTileSizes()
{
  return this->tileSizes;
}
std::string Parallelization::printTransformation()
{

  std::string result = "TP( ";
  // Iterate over the elements of the vector and append them to the string
  for (size_t i = 0; i < (tileSizes).size(); ++i)
  {
    result += std::to_string((tileSizes)[i]);

    if (i != (tileSizes).size() - 1)
    {
      result += ", ";
    }
  }
  result += " )";

  return result;
}
void Parallelization::applyTransformation(CodeIR CodeIr)
{
}

SmallVector<Node *, 2> Parallelization::createParallelizationCandidates(Node *node,
                                                                        mlir::MLIRContext *context)
{
  // Set the maximum number of loops for parallelization (commented out)
  // int64_t maxNumberLoops = 4;

  // Define a vector of possible tile sizes (commented out)
  // std::vector<int64_t> possibleTileSizes = {32, 64, 128};

  // Initialize vectors to store tile combinations 
  SmallVector<SmallVector<int64_t, 4>, 4> tileCombinations;

  // Initialize a list to store child nodes
  SmallVector<SmallVector<Node *, 2>> ChildNodesList;


  // Extract the transformed code IR from the input node
  MLIRCodeIR *CodeIr = (MLIRCodeIR *)node->getTransformedCodeIr();

  // Get the top-level operation
  Operation *target = ((mlir::OwningOpRef<Operation *> *)(*CodeIr)
                           .getIr())
                          ->get();

  // Walk through operations in the target
  target->walk([&](Operation *op)
  {
    // TEMP: Check if the operation is not a "linalg.fill" operation
    if ((op->getName().getStringRef()).str() != "linalg.fill")
    {
      // Check if the operation supports tiling
      if (mlir::TilingInterface tileableOp = dyn_cast<mlir::TilingInterface>(op))
      {

        SmallVector<Node *, 2> ChildNodes;

        OpBuilder builder(context);
        SmallVector<Range> iterationDomain = tileableOp.getIterationDomain(builder);

        llvm::SmallVector<int64_t> upperBounds;
        for (const auto &range : llvm::enumerate(iterationDomain))
        {
          llvm::SmallVector<Value> dynamicVec;
          llvm::SmallVector<int64_t> staticVec;
          dispatchIndexOpFoldResult(range.value().size,
                                    dynamicVec,
                                    staticVec);
          upperBounds.append(staticVec.begin(), staticVec.end());
        }
        SmallVector<SmallVector<int64_t, 4>, 4> possibleTileSizes;

        for (int64_t value : upperBounds) {
            llvm::SmallVector<int64_t, 4> dividers;
            for (int64_t i = 2; i < value; ++i) {
                if (i > 9 && value % i == 0) {
                    dividers.push_back(i);
                }
            }
            possibleTileSizes.push_back(dividers);
        }
        for (int NumberLoops = 2; NumberLoops <= upperBounds.size()-1; ++NumberLoops)
        {
          SmallVector<SmallVector<int64_t, 4>, 4> newCombinations =
              generateTileForAllOpCombinations(NumberLoops, possibleTileSizes, upperBounds);
          tileCombinations.insert(tileCombinations.end(), newCombinations.begin(), newCombinations.end());
        }
   /* std::cout << "Upper BOUNDS\n";
        for (const int64_t value : upperBounds) {
          std::cout << value << " ";
          std::cout << std::endl;
        }
         std::cout << "Deviders\n";
        for (const auto &outerVector : possibleTileSizes) {
          for (const int64_t value : outerVector) {
              std::cout << value << " ";
          }
          std::cout << std::endl;
        }
            std::cout << "Tiling Sizes\n";
        for (const auto &outerVector : tileCombinations) {
          for (const int64_t value : outerVector) {
              std::cout << value << " ";
          }
          std::cout << std::endl;
         }
        std::cout << "End Tiling Sizes\n";*/
        //llvm::SmallVector<int64_t, 4> elementsToInsert = {200, 10};

      // Insert the elements into tileCombinations
      //tileCombinations.push_back(elementsToInsert);
        /*SmallVector<SmallVector<int64_t, 4>, 4> SelectedTileCombinations;
          std::sample(
              tileCombinations.begin(),
              tileCombinations.end(),
              std::back_inserter(SelectedTileCombinations),
              tileCombinations.size()/4,
              std::mt19937{std::random_device{}()}
          );*/
      for (const auto &candidate : tileCombinations)
      {

        MLIRCodeIR *ClonedCode = (MLIRCodeIR *)CodeIr->cloneIr();
        Node *ChildNode = new Node(ClonedCode);

        std::vector<Transformation *> TransList = node->getTransformationList();
        ChildNode->setTransformationList(TransList);

        Parallelization *parallelization =
            new Parallelization(&tileableOp,
                                candidate,
                                context);

        ChildNode->setTransformation(parallelization);

        ChildNode->addTransformation(parallelization);

        ChildNodes.push_back(ChildNode);
      }
      ChildNodesList.push_back(ChildNodes);
    } }
  });
  int OpIndex = 0;
  for (auto ChildNodes : ChildNodesList)
  {
    for (auto node : ChildNodes)
    {
      Operation *ClonedTarget = ((mlir::OwningOpRef<Operation *> *)(*((MLIRCodeIR *)node->getTransformedCodeIr()))
                                     .getIr())
                                    ->get();
      Parallelization *parallelization = (Parallelization *)node->getTransformation();

      int ClonedOpIndex = 0;
      ClonedTarget->walk([&](Operation *op)
                         {
        if (mlir::TilingInterface ClonedTileableOp 
                          = dyn_cast<mlir::TilingInterface>(op)) {
            if ((op->getName().getStringRef()).str() != "linalg.fill" ){
              
        
              
              IRRewriter rewriter(context);
              OpBuilder builder(context);
            
              std::optional<ArrayAttr> mapping;
              SmallVector<OpFoldResult, 4> opFoldResults;
              for (int64_t value : parallelization->getTileSizes()) {
                    opFoldResults.push_back(builder.getIndexAttr(value));
              }
              rewriter.setInsertionPoint(ClonedTileableOp);
              ArrayRef<OpFoldResult> tileSizes =  llvm::makeArrayRef(opFoldResults);
              FailureOr<linalg::ForallTilingResult> tilingResult = 
                          linalg::tileToForallOpUsingTileSizes(rewriter,ClonedTileableOp,tileSizes,mapping);
              //tilingResult->tileOp->dump();
              if (!failed(tilingResult))
                  rewriter.replaceOp(ClonedTileableOp, tilingResult->tileOp->getResults());    
              //IRRewriter rewriter1(context);
              //FuseIntoContainingOperation(tilingResult->tileOp, ClonedTarget, rewriter1);
              
            }
          ClonedOpIndex++;
          } });
          
    }
    OpIndex++;
  }

  SmallVector<Node *, 2> ResChildNodes;
  for (const auto &innerVector : ChildNodesList)
  {
    ResChildNodes.insert(ResChildNodes.end(), innerVector.begin(), innerVector.end());
  }

  return ResChildNodes;
}


// Function to generate tiling sizes that are multiples of the upperBounds.
void generateForAllOpCombinations(const llvm::SmallVector<llvm::SmallVector<int64_t, 4>, 4> &tileSizes,
                               int64_t maxNumberLoops,
                               int64_t currentLoop,
                               llvm::SmallVector<int64_t, 4> &currentCombination,
                               std::vector<llvm::SmallVector<int64_t, 4>> &combinations,
                               const llvm::SmallVector<int64_t> &upperBounds)
{
  if (currentLoop >= maxNumberLoops)
  {
    combinations.push_back(currentCombination);
    return;
  }
  llvm::SmallVector<int64_t, 4> currentTileSizes = tileSizes[currentLoop];

  for (int64_t tileSize : currentTileSizes)
  {
    // Check if the current tileSize is a multiple of the corresponding upperBound.
    if (upperBounds[currentLoop] % tileSize == 0)
    {

      currentCombination[currentLoop] = tileSize;
      generateForAllOpCombinations(tileSizes,
                                maxNumberLoops,
                                currentLoop + 1,
                                currentCombination,
                                combinations,
                                upperBounds);
    }
  }
}

llvm::SmallVector<llvm::SmallVector<int64_t, 4>, 4>
generateTileForAllOpCombinations(int64_t maxNumberLoops,
                              const llvm::SmallVector<llvm::SmallVector<int64_t, 4>, 4> &possibleTileSizes,
                              const llvm::SmallVector<int64_t> &upperBounds)
{

  llvm::SmallVector<int64_t, 4> currentCombination(maxNumberLoops);
  std::vector<llvm::SmallVector<int64_t, 4>> combinations;

  generateForAllOpCombinations(possibleTileSizes,
                            maxNumberLoops,
                            0,
                            currentCombination,
                            combinations,
                            upperBounds);

  return llvm::SmallVector<llvm::SmallVector<int64_t, 4>, 4>(combinations.begin(),
                                                             combinations.end());
}

/*void generateForAllOpCombinations(const SmallVector<int64_t, 4> &tileSizes,
                                  int64_t maxNumberLoops,
                                  int64_t currentLoop,
                                  SmallVector<int64_t, 4> &currentCombination,
                                  std::vector<SmallVector<int64_t, 4>> &combinations)
{
  if (currentLoop >= maxNumberLoops)
  {
    combinations.push_back(currentCombination);
    return;
  }

  for (int64_t tileSize : tileSizes)
  {
    currentCombination[currentLoop] = tileSize;
    generateForAllOpCombinations(tileSizes,
                                 maxNumberLoops,
                                 currentLoop + 1,
                                 currentCombination,
                                 combinations);
  }
}

SmallVector<SmallVector<int64_t, 4>, 4>
generateTileForAllOpCombinations(int64_t maxNumberLoops,
                                 const std::vector<int64_t> &possibleTileSizes)
{
  SmallVector<int64_t, 4> tileSizes;
  std::copy(possibleTileSizes.begin(),
            possibleTileSizes.end(),
            std::back_inserter(tileSizes));

  SmallVector<int64_t, 4> currentCombination(maxNumberLoops);
  std::vector<SmallVector<int64_t, 4>> combinations;

  generateForAllOpCombinations(tileSizes,
                               maxNumberLoops,
                               0,
                               currentCombination,
                               combinations);

  return SmallVector<SmallVector<int64_t, 4>, 4>(combinations.begin(),
                                                 combinations.end());
}*/
