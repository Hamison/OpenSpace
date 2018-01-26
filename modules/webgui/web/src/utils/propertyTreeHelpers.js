export const splitURI = (URI) => {
  const indexForName = URI.indexOf('.');
  const name = URI.substring(0, indexForName !== -1 ?
    indexForName : URI.length);
  const newURI = URI.substring(indexForName + 1, URI.length);

  return { name, URI: newURI, isLastOwner: newURI.indexOf('.') === -1, isLastNode: URI.indexOf('.') === -1 };
};

export const getIdOfProperty = (URI) => {
  const indexForName = URI.lastIndexOf('.');
  return URI.substring(indexForName !== -1 ?
    indexForName + 1 : 0, URI.length);
};

// Function to return a deep copy of an object
export const keepCloning = (objectpassed) => {
  if (objectpassed === null || typeof objectpassed !== 'object') {
    return objectpassed;
  }
  // give temporary-storage the original obj's constructor
  const temporaryStorage = objectpassed.constructor();
  for (const key in objectpassed) {
    temporaryStorage[key] = keepCloning(objectpassed[key]);
  }
  return temporaryStorage;
};

export const traverseTreeWithURI = (node, URI) => {
  const splittedURI = splitURI(URI);
  let tmpValue;
  if (splittedURI.isLastNode) {
    node.properties.forEach((element) => {
      if (element.id === splittedURI.URI) {
        tmpValue = element;
      }
    });
    return tmpValue;
  }
  node.subowners.forEach((element) => {
    if (element.name === splittedURI.name) {
      if (splittedURI.isLastNode) { tmpValue = element; } else {
        tmpValue = traverseTreeWithURI(element, splittedURI.URI);
      }
      return tmpValue;
    }
  });
  return tmpValue;
};

// Conversion from json type array to lua
export const jsonToLuaTable = json => json.replace('[', '').replace(']', '');

export const jsonToLuaString = json => `"${json}"`;


export const traverseTreeForTag = (node, tag) => {
  let data;
  node.subowners.map(element => {
    data = traverseTreeForTag(element, tag);
      if( element.tag.includes(tag)) {
        data = element;
      }
    });
  return data;
};

export const findAllNodesWithTag = (state, tag) => {
  let nodes = [];
  state.map(element => {
      const data = traverseTreeForTag(element, tag);
      if(data !== undefined) {
        const returnValue = {
          data,
          name: element.name
        }
        nodes.push(returnValue);
      }
  })
  return nodes;
};

// Convert envelopes in transfer function property to back end compatible format
export const convertEnvelopes = (envelopes) => {
  let convertedEnvelopes = keepCloning(envelopes);
  convertedEnvelopes = convertedEnvelopes.map(envelope =>
    Object.assign({},
      { points: envelope.points.map(point =>
        Object.assign({},
          { color: point.color,
            position: point.position,
          }),
      ),
      },
    ),
  );
  return JSON.stringify(convertedEnvelopes);
};
